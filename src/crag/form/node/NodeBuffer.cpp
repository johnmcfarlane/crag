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

#include <algorithm>


namespace 
{

	CONFIG_DEFINE (fix_num_quaterna, int, 0);

	// TODO: Shouldn't these just go in Quaterna.h

	// local function definitions
	bool SortByScore(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		return lhs.parent_score > rhs.parent_score;
	}
	
	bool SortByScoreExtreme(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		if (! lhs.IsEasilyExpendable() && rhs.IsEasilyExpendable()) {
			return true;
		}
		
		return SortByScore(lhs, rhs);
	}
	
	bool SortByNodes(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		return lhs.nodes < rhs.nodes;
	}

}


////////////////////////////////////////////////////////////////////////////////
// NodeBuffer functions


form::NodeBuffer::NodeBuffer(int target_num_quaterna)
//: nodes(new Node [max_num_nodes])
: nodes(reinterpret_cast<Node *>(Allocate(sizeof(Node) * max_num_nodes, 128)))
, nodes_used_end(nodes)
, nodes_end(nodes + max_num_nodes)
, quaterna(new Quaterna [max_num_quaterna])
, quaterna_used_end(quaterna)
, quaterna_sorted_end(quaterna)
, quaterna_used_end_target(quaterna + Min(fix_num_quaterna ? fix_num_quaterna : target_num_quaterna, static_cast<int>(max_num_quaterna)))
, quaterna_end(quaterna + max_num_quaterna)
, points(max_num_verts)
#if (USE_OPENCL)
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
	
	VerifyObject(points);
	
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
	VerifyTrue(parent->score >= 0);
	
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

	Quaterna * new_quaterna_used_end = Clamp<Quaterna *>(quaterna + n, quaterna + 0, const_cast<Quaterna *>(quaterna_end));
	
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
void form::NodeBuffer::Tick(Ray3 const & camera_ray_relative)
{
	VerifyObject (* this);	// should fail because of the following.

	// Assuming the camera has moved (and maybe new nodes have been created),
	// the scores for all nodes will be a little different.
	
	// Recalculate the node scores.
	UpdateNodeScores(camera_ray_relative);
	
	// Reflect the new scores in the quaterna.
	UpdateQuaternaScores();

	// Now resort the quaterna so they are in order again.
	SortQuaterna();

	// Finally, using the quaterna,
	// replace nodes whose parent's scores have dropped enough
	// with ones whose score have increased enough.p
	ChurnNodes();
	
	VerifyObject (* this);	// should fail because of the following.
}

void form::NodeBuffer::InitKernel()
{
#if (USE_OPENCL)
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

void form::NodeBuffer::OnReset()
{
	points.FastClear();
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

void form::NodeBuffer::InitQuaterna(Quaterna const * end)
{
#if defined(FAST_SCENE_RESET)
	ZeroArray(nodes, (end - quaterna) * 4);
#endif
	
	Node * n = nodes;
	for (Quaterna * iterator = quaterna; iterator < end; n += 4, ++ iterator) {
		iterator->parent_score = -1;
		iterator->nodes = n;
	}

#if 0
	// Update ranking parent score value.
	for (Quaterna * iterator = quaterna; iterator != quaterna_available_end; ++ iterator) 
	{
		Assert(! iterator->nodes[0].IsInUse());
		Assert(! iterator->nodes[1].IsInUse());
		Assert(! iterator->nodes[2].IsInUse());
		Assert(! iterator->nodes[3].IsInUse());
		
		iterator->parent_score = -1;
	}
#endif
}

void form::NodeBuffer::UpdateNodeScores(Ray3 const & camera_ray_relative)
{
#if (USE_OPENCL)
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
		CalculateNodeScoreFunctor f(camera_ray_relative.position, camera_ray_relative.direction);
		ForEachNode(f);
	}
}

void form::NodeBuffer::UpdateQuaternaScores()
{
	int fetch_ahead = 32;
	
	// Update ranking parent score value.
	for (Quaterna * iterator = quaterna; iterator != quaterna_used_end; ++ iterator) 
	{
		// fetch ahead quaterna->node->parent
		Quaterna * to_fetch = iterator;
		if ((to_fetch += fetch_ahead) < quaterna_used_end) 
		{
			Node * parent = to_fetch->nodes[0].parent;
			if (parent != nullptr) 
			{
				// parent
				PrefetchObject(ref(parent));
			}
			
			if ((to_fetch += fetch_ahead) < quaterna_used_end) 
			{
				// node
				PrefetchObject(to_fetch->nodes[0]);
				
				to_fetch += fetch_ahead;
				if ((to_fetch += fetch_ahead) < quaterna_used_end) 
				{
					// ranking
					PrefetchObject(ref(to_fetch));
				}
			}
		}
		
		Node * parent = iterator->nodes[0].parent;
		if (parent != nullptr) 
		{
			iterator->parent_score = parent->score;
		}
	}
	
	// This basically says: "as far as I know, none of the quaterna are sorted."
	quaterna_sorted_end = quaterna;
}

// Algorithm to sort nodes array. 
// TODO: Exploit is_sorted somehow! 
void form::NodeBuffer::SortQuaterna()
{
	if (quaterna_sorted_end == quaterna_used_end)
	{
		VerifyObject(* this);
		return;
	}
	
#if 1
	// Sure and steady ... and slow!
	std::sort(quaterna, quaterna_used_end, SortByScore);
	quaterna_sorted_end = quaterna_used_end;
	
#else
	// Since we only sort nodes in order to find the lowest-scoring nodes, 
	// it makes sense to only bother ensuring the bottom end of nodes contains
	// all the lowest-scoring nodes in ascening order. Beyond that, they can
	// remain unordered. This algorithm takes advantage of this affordance.
	
	int num_sorted_nodes = Max(static_cast<int>(static_cast<float>(num_rankings_recycled_this_tick) * recycle_to_sorted_coefficient) + 1, min_sorted_nodes);
	
	Node * * const sorted_begin = rankings_begin;
	Node * * const sorted_end = Min(sorted_begin + num_sorted_nodes, quaterna_available_end);
	
	// Sort the sorted portion nodes amongst themselves.
	std::sort(sorted_begin, sorted_end, NodeComp);
	
	// Now ensure that nodes beyond
	Node * * const unsorted_begin = sorted_end;
	Node * const * const unsorted_end = quaterna_available_end;
	
	Node * const * const last_sorted = sorted_end - 1;
	float last_sorted_score = (* last_sorted)->score;
	
	for (Node * * iterator = unsorted_begin; iterator < unsorted_end; ++ iterator)
	{
		Node * const node = * iterator;
		float score = node->score;
		
		// If this node is higher score than any in the sorted portion,
		Assert((* last_sorted)->score == last_sorted_score);
		if (score >= last_sorted_score)
		{
			// then perform a mini-bubble on this and the previous node.
			// Slowly, the entire array should tend towards sorted - which can only help. 
			// TODO: cache iterator[-1]->score
			if (score < iterator[-1]->score)
			{
				iterator[0] = iterator[-1];
				iterator[-1] = node;
				last_sorted_score = (* last_sorted)->score;
			}
			
			continue;
		}
		// The node belongs in the sorted range.
		
		// Find the last position in the sorted range where node could go.
		Node * * result = std::upper_bound(sorted_begin, sorted_end, * iterator, NodeComp);
		Assert(result >= sorted_begin && result < sorted_end);
		
		// They all rolled over and one fell out.
		(* iterator) = (* last_sorted);
		
		// Roll over, roll over.
		Node * * const src = result;
		Node * * const dst_begin = result + 1;
		Node * const * const dst_end = sorted_end;
		int count = dst_end - dst_begin;
		memmove(dst_begin, src, count * sizeof(* nodes));
		
		// Finally, the little one. 
		(* src) = node;
		
		last_sorted_score = (* last_sorted)->score;
	}
#endif
	
//	num_rankings_recycled += num_rankings_recycled_this_tick;
//	num_rankings_recycled_this_tick = 0;
}

bool form::NodeBuffer::ChurnNodes()
{
	// TODO: ForEachQuaterna is what I really want.
	// TODO: This will expand the nodes in an improved order.

	ExpandNodeFunctor f(* this);
	ForEachNode(f);
	
	return f.GetNumExpanded() > 0;
}

void form::NodeBuffer::GenerateMesh(Mesh & mesh) 
{
	//int fetch_ahead = 32;
	
	points.Clear();
	
	gfx::IndexBuffer & indices = mesh.GetIndices();
	indices.Clear();
	
	VertexBuffer & vertices = mesh.GetVertices();
	vertices.Clear();
	
	//mesh.SetVertices(& points);
	
	{
		GenerateMeshFunctor f(mesh);
		ForEachNode(f);
	}
	
	vertices.NormalizeNormals();
}


///////////////////////////////////////////////////////
// Node-related members.

bool form::NodeBuffer::ExpandNode(Node & node) 
{
	Assert(node.IsExpandable());
	
	// Now find the worst quaterna and attempt to reuse it as node's new children.
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
	}
	else if (quaterna_sorted_end > quaterna) 
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
			// TODO: If we get here, it's probably a great idea to early-out of the enture churn function.
			return false;
		}
		
		if (! ExpandNode(node, reusable_quaterna))
		{
			return false;
		}
		
		-- quaterna_sorted_end;
	}
	else
	{
		Assert(false);
		return false;
	}
	
	return true;
}
	
bool form::NodeBuffer::ExpandNode(Node & node, Quaterna & children_quaterna)	
{
	Node * worst_children = children_quaterna.nodes;
	Assert(worst_children + 0 != & node);
	Assert(worst_children + 1 != & node);
	Assert(worst_children + 2 != & node);
	Assert(worst_children + 3 != & node);
		
	Shader & shader = GetPolyhedron(node).GetShader();
	node.InitMidPoints(points, shader);
	
	// Work with copy of children until it's certain that expansion is going to work.
	Node children_copy[4];	
	if (! InitChildGeometry(node, children_copy, shader)) 
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

bool form::NodeBuffer::InitChildGeometry(Node const & parent, Node * children, Shader & shader)
{
#if 1
	Node::Triplet const * parent_triple = parent.triple;
	
	Node * child = children;
	Node::Triplet * child_triple;
	
	child_triple = child->triple;
	child_triple->corner = parent_triple[0].corner;
	(++ child_triple)->corner = parent_triple[2].mid_point;
	(++ child_triple)->corner = parent_triple[1].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple->corner = parent_triple[2].mid_point;
	(++ child_triple)->corner = parent_triple[1].corner;
	(++ child_triple)->corner = parent_triple[0].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple->corner = parent_triple[1].mid_point;
	(++ child_triple)->corner = parent_triple[0].mid_point;
	(++ child_triple)->corner = parent_triple[2].corner;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple->corner = parent_triple[0].mid_point;
	(++ child_triple)->corner = parent_triple[1].mid_point;
	(++ child_triple)->corner = parent_triple[2].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
#else
	
	// TODO: Hard-code this and remove GetChildCorners.
	for (int child_index = 0; child_index < 4; ++ child_index) 
	{
		Point * child_corners[3];
		parent.GetChildCorners(child_index, child_corners);
		
		Node & child = children [child_index];
		
		child.triple[0].corner = child_corners[0];
		child.triple[1].corner = child_corners[1];
		child.triple[2].corner = child_corners[2];
		if (child.InitScoreParameters() == false) {
			return false;
		}
		
		child.score = 0;	// std::numeric_limits<float>::max();
	}
#endif
	
	Assert(parent.triple[0].corner == children[0].triple[0].corner);
	
	return true;
}

// Make sure cousins and their mid-points match up.
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
				points.Free(t.mid_point);
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
	// The target pointer now points into the range of unused quaterna at the end of the array.
	quaterna_used_end_target = new_quaterna_used_end;
}

// TODO: This can be improved a lot.
// TODO: Should sweep all surviving quats first and do all the substitutions in the correct order.
// TODO: That way, the unavailable quats can keep their node pointers.
void form::NodeBuffer::DecreaseNodes(Quaterna * new_quaterna_used_end)
{
	Quaterna * old_quaterna_used_end = quaterna_used_end;
	
	DecreaseQuaterna(new_quaterna_used_end);
	
	// Was there any decrease at all?
	if (old_quaterna_used_end == quaterna_used_end)
	{
		Assert(false);	// This isn't deadly fatal but serious enough that I'd like to know it happens.
		return;
	}
	
	FixUpDecreasedNodes(old_quaterna_used_end);

	std::sort(quaterna_used_end, old_quaterna_used_end, SortByNodes);
	
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
				int n = new_quaterna_used_end - quaterna;
				int m = quaterna_used_end - quaterna;
				std::cout << m << ":" << n << '\n';
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

template <class FUNCTOR> void form::NodeBuffer::ForEachNode(FUNCTOR & f, int step_size)
{
	ForEach(f, nodes, nodes_used_end, step_size);
}

