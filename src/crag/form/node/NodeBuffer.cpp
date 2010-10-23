/*
 *  NodeBuffer.cpp
 *  Crag
 *
 *  Created by John on 2/17/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "NodeBuffer.h"

#include "ExpandNodeFunctor.h"
#include "GenerateMeshFunctor.h"
#include "Node.h"
#include "Quaterna.h"
#include "Shader.h"

#include "form/scene/Mesh.h"
#include "form/scene/Polyhedron.h"

#include "form/score/CalculateNodeScoreFunctor.h"

#include "cl/Singleton.h"

#include "core/ConfigEntry.h"
#include "core/floatOps.h"
#include "core/for_each_chunk.h"

#include <algorithm>

// Note: Doesn't work yet.
#define EXPAND_NODES_PARALLEL false

#define PREFETCH_ARRAYS false


namespace 
{

	// If non-zero, this number of quaterna is enforced always.
	// It is useful for eliminating the adaptive quaterna count algorithm during debugging.
	// TODO: Force CONFIG_DEFINE & pals to include a description in the .cfg file.
	CONFIG_DEFINE (fix_num_quaterna, int, 0);

	
	////////////////////////////////////////////////////////////////////////////////
	// local function definitions

	inline void UpdateQuaternaScore(form::Quaterna & q) 
	{
		form::Node * parent = q.nodes[0].parent;
		if (parent != nullptr) 
		{
			q.parent_score = parent->score;
		}
	}
		
	bool QuaternaSortUsed(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		return lhs.parent_score > rhs.parent_score;
	}
	
	bool QuaternaSortUnused(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		return lhs.nodes < rhs.nodes;
	}

}


////////////////////////////////////////////////////////////////////////////////
// NodeBuffer functions

form::NodeBuffer::NodeBuffer()
//: nodes(new Node [max_num_nodes])
: nodes(reinterpret_cast<Node *>(Allocate(sizeof(Node) * max_num_nodes, 128)))
, nodes_used_end(nodes)
, nodes_end(nodes + max_num_nodes)
, quaterna(new Quaterna [max_num_quaterna])
, quaterna_sorted_end(quaterna)
, quaterna_used_end(quaterna)
, quaterna_used_end_target(quaterna + Min(fix_num_quaterna ? fix_num_quaterna : 0, static_cast<int>(max_num_quaterna)))
, quaterna_end(quaterna + max_num_quaterna)
, point_buffer(max_num_verts)
, cached_node_score_ray(CalculateNodeScoreFunctor::GetInvalidRay())
#if defined(USE_OPENCL)
, cpu_kernel(nullptr)
, gpu_kernel(nullptr)
#endif
{
	InitKernel();
	
	ZeroArray(nodes, max_num_nodes);

	InitQuaterna(quaterna_end);
	
	VerifyObject(* this);
}

form::NodeBuffer::~NodeBuffer()
{
	VerifyObject(* this);

	Assert(GetNumQuaternaUsed() == 0);
	
	delete quaterna;

	//delete nodes;
	Free(nodes);
}

#if VERIFY
void form::NodeBuffer::Verify() const
{
	VerifyArrayElement(nodes_used_end, nodes, nodes_end + 1);	
	
	VerifyArrayElement(quaterna_sorted_end, quaterna);
	VerifyArrayElement(quaterna_used_end, quaterna);
	VerifyArrayElement(quaterna_used_end_target, quaterna);
	VerifyArrayElement(quaterna_end, quaterna);
	
	VerifyTrue(quaterna_sorted_end >= quaterna);
	VerifyTrue(quaterna_used_end >= quaterna_sorted_end);
	VerifyTrue(quaterna_used_end_target >= quaterna_used_end);
	VerifyTrue(quaterna_end >= quaterna_used_end_target);

	int num_nodes_used = nodes_used_end - nodes;
	int num_quaterna_used = quaterna_used_end - quaterna;

	VerifyTrue(num_nodes_used == num_quaterna_used * 4);
	
	VerifyObject(point_buffer);
	
/*	for (Quaterna const * q = quaterna; q < quaterna_used_end; ++ q) 
	{
		VerifyUsed(* q);
	}
	
	// Rarely cause a problem and they are often the majority.
	for (Quaterna const * q = quaterna_used_end; q < quaterna_end; ++ q) 
	{
		VerifyUnused(* q);
	}
	
	for (Quaterna const * q2 = quaterna + 1; q2 < quaterna_sorted_end; ++ q2) 
	{
		Quaterna const * q1 = q2 - 1;
		VerifyTrue(q2->parent_score <= q1->parent_score);
	}*/
}

void form::NodeBuffer::VerifyUsed(Quaterna const & q) const
{
	Node const * n = q.nodes;
	VerifyArrayElement(n, nodes, nodes_used_end);
	
	Node const * parent = q.nodes[0].parent;
	
	VerifyTrue(parent != nullptr);
	VerifyTrue(parent->score == q.parent_score);
	VerifyTrue(parent->score > 0);
	
	for (int i = 0; i < 4; ++ i)
	{
		form::Node const & sibling = q.nodes[i];
		
		// All four siblings should have the same parent.
		VerifyTrue(q.nodes[i].parent == parent);
		if (sibling.children != nullptr)
		{
			VerifyArrayElement(sibling.children, nodes, nodes_used_end);
		}
	}
	
	VerifyObject(q);
}

void form::NodeBuffer::VerifyUnused(Quaterna const & q) const
{
	VerifyTrue(q.parent_score == -1);
	
	Node const * n = q.nodes;
	VerifyTrue(n - nodes == (& q - quaterna) * 4);
	
	for (int i = 0; i < 4; ++ i)
	{
		form::Node const & sibling = q.nodes[i];
		
		VerifyTrue(sibling.parent == nullptr);
		VerifyTrue(sibling.children == nullptr);
		VerifyTrue(sibling.score == 0);
	}
	
	VerifyObject(q);
}
#endif

#if DUMP
DUMP_OPERATOR_DEFINITION(form, NodeBuffer)
{
	for (Quaterna * r = rhs.quaterna; r != rhs.quaterna_available_end; ++ r) {
		lhs << lhs.NewLine() << "p_score:" << r->parent_score << "; children:" << r->nodes;
		DumpStream indented = lhs;
		DumpStream indented_more = indented;
		for (int i = 0; i < 4; ++ i) {
			Node * n = r->nodes + i;
			indented << indented.NewLine() << "node" << i << ":" << n;
			indented_more << * n;
		}
	}
	
	return lhs;
}
#endif

int form::NodeBuffer::GetNumNodesUsed() const
{
	return nodes_used_end - nodes;
}

int form::NodeBuffer::GetNumQuaternaUsed() const
{
	return quaterna_used_end - quaterna;
}

int form::NodeBuffer::GetNumQuaternaUsedTarget() const
{
	return quaterna_used_end_target - quaterna;
}

void form::NodeBuffer::SetNumQuaternaUsedTarget(int n)
{
	if (fix_num_quaterna != 0)
	{
		return;
	}

	Quaterna * new_quaterna_used_end = quaterna + n;
	VerifyArrayElement(new_quaterna_used_end, quaterna, quaterna_end);
	
	if (new_quaterna_used_end == quaterna_used_end) 
	{
		// No change. Just make sure the target is the same
		quaterna_used_end_target = new_quaterna_used_end;
	}
	else if (new_quaterna_used_end > quaterna_used_end) 
	{
		IncreaseNodes(new_quaterna_used_end);
	}
	else if (new_quaterna_used_end < quaterna_used_end)
	{
		LockTree();
		DecreaseNodes(new_quaterna_used_end);
		UnlockTree();
	}
}

void form::NodeBuffer::LockTree() const
{
	tree_mutex.Lock();
}

void form::NodeBuffer::UnlockTree() const
{
	tree_mutex.Unlock();
}

// This is the main tick function for all things 'nodey'.
// It is also where a considerable amount of the SceneThread's time is spent.
void form::NodeBuffer::Tick(Ray3 const & new_camera_ray)
{
	VerifyObject (* this);	// should fail because of the following.

	node_score_functor.SetCameraRay(new_camera_ray);

	// Is the new camera ray significantly different to 
	// the one used to last score the bulk of the node buffer?
	if (node_score_functor.IsSignificantlyDifferent(cached_node_score_ray))
	{
		UpdateNodeScores();
		cached_node_score_ray = new_camera_ray;
	}
	
	UpdateNodes();
}

void form::NodeBuffer::OnReset()
{
	point_buffer.FastClear();
	InitQuaterna(quaterna_used_end);

	nodes_used_end = nodes;
	quaterna_sorted_end = quaterna_used_end = quaterna;
	
	// Half the target number of nodes.
	// Probably not a smart idea.
	//quaterna_used_end_target -= (quaterna_used_end_target - quaterna) >> 1;
}

void form::NodeBuffer::ResetNodeOrigins(Vector3 const & origin_delta)
{
	for (Node * node = nodes; node != nodes_used_end; ++ node)
	{
		Node::Triplet * triple = node->triple;
		for (int i = 0; i < 3; ++ i)
		{
			Node::Triplet & t = triple[i];
			if (t.mid_point != nullptr)
			{
				// We only want to increment each mid-point once, 
				// but it's often pointed to by two nodes.
				// This is an arbitrary test to ensure it's only incremented for one node.
				// Note that if cousin is null, the condition still passes. 
				if (node > t.cousin)
				{
					t.mid_point->pos -= origin_delta;
				}
			}
		}
		
		node->center += origin_delta;
	}
}

void form::NodeBuffer::InitKernel()
{
#if defined(USE_OPENCL)
	cl::Singleton const & cl_singleton = cl::Singleton::Get();
	
	switch (cl_singleton.GetDeviceType())
	{
		case CL_DEVICE_TYPE_CPU:
			cpu_kernel = new CalculateNodeScoreCpuKernel(max_num_nodes, nodes);
			break;
			
		case CL_DEVICE_TYPE_GPU:
			gpu_kernel = new CalculateNodeScoreGpuKernel(max_num_nodes);
			break;
	}
#endif
}

void form::NodeBuffer::InitQuaterna(Quaterna const * end)
{
	Node * n = nodes;
	for (Quaterna * iterator = quaterna; iterator < end; n += 4, ++ iterator) {
		iterator->parent_score = -1;
		iterator->nodes = n;
	}
}

void form::NodeBuffer::UpdateNodes()
{
	do 
	{
		// Reflect the new scores in the quaterna.
		UpdateQuaternaScores();
		
		// Now resort the quaterna so they are in order again.
		SortQuaterna();
		
		// Finally, using the quaterna,
		// replace nodes whose parent's scores have dropped enough
		// with ones whose score have increased enough.
		if (! ChurnNodes())
		{
			break;
		}
	}
	while (IsNodeChurnIntensive());

	VerifyObject (* this);
}

void form::NodeBuffer::UpdateNodeScores()
{
#if defined(USE_OPENCL)
	if (cpu_kernel != nullptr)
	{
		cpu_kernel->Process(nodes, nodes_used_end, camera_ray_relative.position, camera_ray_relative.direction);
	}
	else if (gpu_kernel != nullptr)
	{
		gpu_kernel->Process(nodes, nodes_used_end, camera_ray_relative.position);
	}
	else 
#endif
	{
		ForEachNode<CalculateNodeScoreFunctor &>(1024, node_score_functor, true);
	}
}

void form::NodeBuffer::UpdateQuaternaScores()
{
	ForEachQuaterna(128, UpdateQuaternaScore, true);
	
	// This basically says: "as far as I know, none of the quaterna are sorted."
	quaterna_sorted_end = quaterna;
}

// Algorithm to sort nodes array. 
void form::NodeBuffer::SortQuaterna()
{
	if (quaterna_sorted_end == quaterna_used_end)
	{
		VerifyObject(* this);
		return;
	}
	
	// Sure and steady ... and slow!
	std::sort(quaterna, quaterna_used_end, QuaternaSortUsed);
	quaterna_sorted_end = quaterna_used_end;
}

bool form::NodeBuffer::ChurnNodes()
{
	// TODO: ForEachQuaterna is what I really want.
	// TODO: This will expand the nodes in an improved order.

	ExpandNodeFunctor f(* this);
	//ForEachNode<ExpandNodeFunctor &>(512, f, EXPAND_NODES_PARALLEL);
	ForEachQuaterna<ExpandNodeFunctor &>(512, f, EXPAND_NODES_PARALLEL);
	
	return f.GetNumExpanded() > 0;
}

void form::NodeBuffer::GenerateMesh(Mesh & mesh) 
{
	point_buffer.Clear();
	mesh.Clear();

	GenerateMeshFunctor mesh_functor(mesh);

#if PREFETCH_ARRAYS
	typedef void (* PrefetchFunctor) (Node & node);
	PrefetchFunctor mesh_prefetch_functor = GenerateMeshPrefetchFunctor;
	ForEachNode<PrefetchFunctor &, GenerateMeshFunctor &>(512, mesh_prefetch_functor, mesh_functor, true);
#else
	ForEachNode<GenerateMeshFunctor &>(512, mesh_functor, true);
#endif
	
	// TODO: Parallelize.
	VertexBuffer & vertices = mesh.GetVertices();
	vertices.NormalizeNormals();
}

///////////////////////////////////////////////////////
// Node-related members.

bool form::NodeBuffer::ExpandNode(Node & node) 
{
	Assert(node.IsExpandable());

	// Try and get an unused quaterna.
	if (quaterna_used_end != quaterna_used_end_target)
	{
		// We currently wish to expand the number of nodes/quaterna used,
		// so attempt to use the next unused quaterna in the array.
		Quaterna & unused_quaterna = * quaterna_used_end;

		if (! ExpandNode(node, unused_quaterna))
		{
			return false;
		}
		
		nodes_used_end += 4;
		++ quaterna_used_end;
		return true;
	}
	
	// Find the lowest-scoring quaterna in used and attempt to reuse it.
	if (quaterna_sorted_end > quaterna) 
	{
		// Get the score to use to find the 'worst' Quaterna.
		// We don't want to nuke a quaterna with a worse one, hence using the node score.
		// But also, there's a slim chance that node's parent has a worse score than node's
		// and we can't end up finding a quaterna that includes node as node's new children
		// because that would cause a time paradox in the fabric of space.
		float score = node.score;
		
		// Ok, lets try the end of the sequence of sorted quaterna...
		Quaterna & reusable_quaterna = quaterna_sorted_end [- 1];			
		
		if (! reusable_quaterna.IsSuitableReplacement(score)) 
		{
			return false;
		}

		// Make sure that the node isn't replacing itself or one of its ancestors.
		for (Node * ancestor = & node; ancestor != nullptr; ancestor = ancestor->parent)
		{
			if (ancestor >= reusable_quaterna.nodes && ancestor < reusable_quaterna.nodes + 4)
			{
				return false;
			}
		}
		
		if (! ExpandNode(node, reusable_quaterna))
		{
			return false;
		}
		
		-- quaterna_sorted_end;
		return true;
	}

	// We're clean out of nodes!
	// Unless the node tree is somehow eating itself,
	// this probably means a root nodes is expanding
	// and there are no nodes. 
	return false;
}

bool form::NodeBuffer::ExpandNode(Node & node, Quaterna & children_quaterna)	
{
	Node * worst_children = children_quaterna.nodes;
	Assert(worst_children + 0 != & node);
	Assert(worst_children + 1 != & node);
	Assert(worst_children + 2 != & node);
	Assert(worst_children + 3 != & node);
	
	// Note that after this point, expansion may fail but the node may have new mid-points.
	Shader & shader = GetPolyhedron(node).GetShader();
	if (! node.InitMidPoints(point_buffer, shader))
	{
		return false;
	}
	
	// Work with copy of children until it's certain that expansion is going to work.
	Node children_copy[4];	
	if (! Node::InitChildCorners(node, children_copy)) 
	{
		// Probably, a child is too small to be represented using float accuracy.
		return false;
	}
	
	LockTree();
	
	// Deinit children.
	bool is_in_use = children_quaterna.IsInUse();
	if (is_in_use) 
	{
		DeinitChildren(worst_children);
	}
	Assert(worst_children[0].score == 0);
	Assert(worst_children[1].score == 0);
	Assert(worst_children[2].score == 0);
	Assert(worst_children[3].score == 0);
	
	worst_children[0] = children_copy[0];
	worst_children[1] = children_copy[1];
	worst_children[2] = children_copy[2];
	worst_children[3] = children_copy[3];

	node.children = worst_children;
	InitChildPointers(node);
	
	UnlockTree();
	
	children_quaterna.parent_score = node.score;
	
	// If NodeBuffer::Tick iterates over the nodes multiple times, the new-comers need their scores corrected.
	// To call UpdateNodeScores would be inefficient as it scores every used node. 
	// This way, we only score what is out of date.
	if (IsNodeChurnIntensive())
	{
		node_score_functor (children_quaterna.nodes[0]);
		node_score_functor (children_quaterna.nodes[1]);
		node_score_functor (children_quaterna.nodes[2]);
		node_score_functor (children_quaterna.nodes[3]);
	}
	
	return true;
}

void form::NodeBuffer::CollapseNode(Node & node)
{
	Node * children = node.children;
	if (children != nullptr) {
		DeinitChildren(children);
		node.children = nullptr;
	}
}

// Make sure cousins and their mid-point_buffer match up.
// Also make sure all children know who their parent is.
void form::NodeBuffer::InitChildPointers(Node & parent_node)
{
	Node::Triplet const * parent_triple = parent_node.triple;
	Node * nodes = parent_node.children;
	Node & center = nodes[3];
	
	for (int i = 0; i < 3; ++ i)
	{
		Node & node = nodes[i];

		// This is the side which the child shares with its sibling in the center.
		// anteroposterior
		Assert(node.triple[i].corner == parent_triple[i].corner);
		Assert(node.triple[i].mid_point == nullptr);
		node.SetCousin(i, center);
		
		int j = TriMod(i + 1);
		int k = TriMod(i + 2);
		
		// The other two corners are shared with children of the parent's children.

		// dextrosinister #1
		Assert(node.triple[j].corner == parent_triple[k].mid_point);
		Node * parent_cousin_j = parent_triple[j].cousin;
		if (parent_cousin_j != nullptr) {
			Node * parent_cousin_children_j = parent_cousin_j->children;
			if (parent_cousin_children_j != nullptr) {
				node.SetCousin(j, parent_cousin_children_j[k]);
			}
		}
		
		// dextrosinister #2
		Assert(node.triple[k].corner == parent_triple[j].mid_point);
		Node * parent_cousin_k = parent_triple[k].cousin;
		if (parent_cousin_k != nullptr) {
			Node * parent_cousin_children_k = parent_cousin_k->children;
			if (parent_cousin_children_k != nullptr) {
				node.SetCousin(k, parent_cousin_children_k[j]);
			}
		}
		
		node.seed = parent_node.GetChildSeed(i);
		node.parent = & parent_node;
	}
	
	center.seed = parent_node.GetChildSeed(3);
	center.parent = & parent_node;
}

void form::NodeBuffer::DeinitChildren(Node * children)
{
	Assert(children->parent->children == children);
	children->parent->children = nullptr;
	
	DeinitNode(children[0]);
	DeinitNode(children[1]);
	DeinitNode(children[2]);
	DeinitNode(children[3]);
}

// Nulls all the relevant pointers, disconnects cousins, frees verts.
void form::NodeBuffer::DeinitNode(Node & node)
{
	CollapseNode(node);
	
	for (int i = 0; i < 3; ++ i)
	{
		Node::Triplet & t = node.triple[i];
		
		t.corner = nullptr;
		
		if (t.cousin != nullptr)
		{
			Node::Triplet & mirror = t.cousin->triple[i];
			Assert(mirror.cousin == & node);
			Assert(mirror.mid_point == t.mid_point);
			
			t.cousin = nullptr;
			t.mid_point = nullptr;
			
			mirror.cousin = nullptr;
		}
		else
		{
			// there is no cousin
			if (t.mid_point != nullptr)
			{
				// and there is a mid-point, so delete it.
				point_buffer.Free(t.mid_point);
				t.mid_point = nullptr;
			}
		}
		
		Assert(t.mid_point == nullptr);
	}
	
	node.parent = nullptr;
	node.score = 0;
}

void form::NodeBuffer::SubstituteChildren(Node * substitute, Node * original)
{
	Node * parent = original->parent;
	
	if (parent != nullptr) 
	{
		Assert(parent->children == original);
		parent->children = substitute;
		
		substitute[0] = original[0];
		RepairChild(substitute[0]);
		VerifyObject(substitute[0]);
		
		substitute[1] = original[1];
		RepairChild(substitute[1]);
		VerifyObject(substitute[1]);
		
		substitute[2] = original[2];
		RepairChild(substitute[2]);
		VerifyObject(substitute[2]);
		
		substitute[3] = original[3];
		RepairChild(substitute[3]);
		VerifyObject(substitute[3]);
	}
	else {
		substitute[0] = original[0];
		substitute[1] = original[1];
		substitute[2] = original[2];
		substitute[3] = original[3];
	}
	
	ZeroArray(original, 4);
}

// Repair the pointers that point TO this node.
void form::NodeBuffer::RepairChild(Node & child)
{
	// Repair children's parent pointers.
	Node * node_it = child.children;
	if (node_it != nullptr) {
		Node * node_end = node_it + 4;
		do {
			Assert(node_it->parent != & child);
			node_it->parent = & child;
			++ node_it;
		}	while (node_it < node_end);
	}

	// Repair cousin pointers.
	Node::Triplet * triple = child.triple;
	for (int i = 0; i < 3; ++ i) {
		Node * cousin = triple[i].cousin;
		if (cousin != nullptr) {
			Assert(cousin->triple[i].cousin != nullptr);
			Assert(cousin->triple[i].cousin != & child);
			cousin->triple[i].cousin = & child;
		}
	}
}

void form::NodeBuffer::IncreaseNodes(Quaterna * new_quaterna_used_end)
{
	// Verify that the input is indeed a decrease.
	VerifyArrayElement(new_quaterna_used_end, quaterna_used_end, quaterna_end + 1);
	
	// Increasing the target number of nodes is simply a matter of setting a value. 
	// The target pointer now point_buffer into the range of unused quaterna at the end of the array.
	quaterna_used_end_target = new_quaterna_used_end;
}

// Reduce the number of used nodes and, accordingly, the number of used quaterna.
// This is quite an involved and painful process which sometimes fails half-way through.
void form::NodeBuffer::DecreaseNodes(Quaterna * new_quaterna_used_end)
{
	Quaterna * old_quaterna_used_end = quaterna_used_end;

	// First decrease the number of quaterna. This is the bit that sometimes fails.
	DecreaseQuaterna(new_quaterna_used_end);
	
	// Was there any decrease at all?
	if (old_quaterna_used_end == quaterna_used_end)
	{
		//Assert(false);	// This isn't deadly fatal but serious enough that I'd like to know it happens.
		return;
	}
	
	// Because the nodes aren't in the correct order, decreasing them is somewhat more tricky.
	FixUpDecreasedNodes(old_quaterna_used_end);

	std::sort(quaterna_used_end, old_quaterna_used_end, QuaternaSortUnused);
	
	VerifyObject (* this);	// should fail because of the following.
}

// The first state of reducing the number of used nodes - and quaterna - is
// to 
void form::NodeBuffer::DecreaseQuaterna(Quaterna * new_quaterna_used_end)
{
	// Verify that the input is indeed a decrease.
	VerifyArrayElement(new_quaterna_used_end, quaterna, quaterna_used_end + 1);
	
	// Loop through used quats backwards from far end.
	do 
	{
		Quaterna & q = quaterna_used_end [- 1];
		Node * nodes = q.nodes;
		
		// Is the quaterna is being used?
		if (q.IsInUse()) 
		{
			// Is it a leaf, i.e. it is the quaterna associated with a grandparent?
			if (! q.IsLeaf()) 
			{
				// If so, we cannot easily remove this quaterna.
				// It's best to stop the reduction at this element.
				break;
			}
			
			DeinitChildren(nodes);
			
			// It is no longer being used.
			q.parent_score = -1;
		}

		// Either way, there should be no children.
		Assert(nodes[0].children == 0);
		Assert(nodes[1].children == 0);
		Assert(nodes[2].children == 0);
		Assert(nodes[3].children == 0);
		
		-- quaterna_used_end;
	}	
	while (quaterna_used_end > new_quaterna_used_end);
	
	// Both quaterna_sorted_end and quaterna_used_end_target 
	// must be equal to quaterna_used_end at this point. 
	if (quaterna_sorted_end > quaterna_used_end)
	{
		quaterna_sorted_end = quaterna_used_end;
	}
	
	// Target is really for increasing the target during churn.
	quaterna_used_end_target = quaterna_used_end;
}

void form::NodeBuffer::FixUpDecreasedNodes(Quaterna * old_quaterna_used_end)
{
	// Remember where the used nodes used to end.
//	Node * old_nodes_used_end = nodes_used_end;
	
	// From the new used quaterna value, figure out new used node value.
	int new_num_quaterna_used = quaterna_used_end - quaterna;
	int new_num_nodes_used = new_num_quaterna_used << 2;	// 4 nodes per quaterna
	nodes_used_end = nodes + new_num_nodes_used;
	VerifyArrayElement(nodes_used_end, nodes, nodes_end + 1);
	
	// Use this pointer to walk down the used quaterna array.
	Quaterna * used_quaterna = quaterna_used_end;
	
	// For all the quaterna that have been freed up by the reduction...
	for (Quaterna * unused_quaterna = old_quaterna_used_end - 1; unused_quaterna >= quaterna_used_end; -- unused_quaterna)
	{
		Assert(! unused_quaterna->IsInUse());
		
		Node * nodes = unused_quaterna->nodes;
		
		// Is a quatern past the end of used quaterna
		// pointing to a node before the end of used nodes?
		if (nodes < nodes_used_end)
		{
			// It needs to point to a quad of nodes past the end of used nodes.
			// There must be a corresponding quad of nodes in such a position that is in use.
			// They must be swapped.
			
			// Find the used quaterna whose nodes are past the end of the used range.
			Node * substitute_nodes;
			do
			{
				-- used_quaterna;
				Assert(used_quaterna >= quaterna);
				substitute_nodes = used_quaterna->nodes;
			}
			while (substitute_nodes < nodes_used_end);

			SubstituteChildren(nodes, substitute_nodes);
			std::swap(used_quaterna->nodes, unused_quaterna->nodes);
		}
	}

#if ! defined(NDEBUG)
	while ((-- used_quaterna) >= quaterna)
	{
		Assert(used_quaterna->nodes != nullptr);
	}
#endif
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachNode(size_t step_size, FUNCTOR f, bool parallel)
{
	if (nodes_used_end > nodes)
	{
		core::for_each<form::Node *, FUNCTOR, 4>(nodes, nodes_used_end, step_size, f, parallel, PREFETCH_ARRAYS);
	}
}

template <typename FUNCTOR1, typename FUNCTOR2> 
void form::NodeBuffer::ForEachNode(size_t step_size, FUNCTOR1 f1, FUNCTOR2 f2, bool parallel)
{
	if (nodes_used_end > nodes)
	{
		core::for_each<form::Node *, FUNCTOR1, FUNCTOR2, 4>(nodes, nodes_used_end, step_size, f1, f2, parallel, PREFETCH_ARRAYS);
	}
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachQuaterna(size_t step_size, FUNCTOR f, bool parallel)
{
	if (quaterna_used_end > quaterna)
	{
		// TODO: Consider fixing the number of quaterna to multiples of N to see if there's an unrolling advantage.
		core::for_each<form::Quaterna *, FUNCTOR, 1>(quaterna, quaterna_used_end, step_size, f, parallel, PREFETCH_ARRAYS);
	}
}
