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
#include "Quaterna.h"
#include "Shader.h"

// TODO: Remove dependency on Polyhedron
#include "form/scene/Polyhedron.h"

#include "core/ConfigEntry.h"
#include "core/for_each_chunk.h"


namespace 
{

	// If non-zero, this number of quaterna is enforced always.
	// It is useful for eliminating the adaptive quaterna count algorithm during debugging.
	// TODO: Force CONFIG_DEFINE & pals to include a description in the .cfg file.
#if defined(PROFILE)
	CONFIG_DEFINE (fix_num_quaterna, int, 10000);
#else
	// Don't ever change this value from zero!!!
	CONFIG_DEFINE (fix_num_quaterna, int, 10000);
#endif
	
	
	////////////////////////////////////////////////////////////////////////////////
	// local function definitions

	inline void UpdateQuaternaScore(form::Quaterna & q) 
	{
		form::Node * parent = q.nodes[0].GetParent();
		q.parent_score = (parent != nullptr) ? parent->score : -1;
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
{
	ZeroArray(nodes, max_num_nodes);

	InitQuaterna(quaterna_end);

	VerifyObject(* this);
}

form::NodeBuffer::~NodeBuffer()
{
	VerifyObject(* this);

#if ! defined(NDEBUG)
	for (Quaterna const * i = quaterna; i != quaterna_used_end; ++ i)
	{
		Assert(! i->HasGrandChildren());
		Assert(! i->nodes[0].IsInUse());
		Assert(! i->nodes[1].IsInUse());
		Assert(! i->nodes[2].IsInUse());
		Assert(! i->nodes[3].IsInUse());
	}
#endif
	
	delete quaterna;

	//delete nodes;
	Free(nodes);
}

#if defined(VERIFY)
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
	VerifyTrue((num_nodes_used % portion_num_nodes) == 0);
	
	int num_quaterna_used = quaterna_used_end - quaterna;
	VerifyTrue((num_quaterna_used % portion_num_quaterna) == 0);
	
	int num_quaterna_used_target = quaterna_used_end_target - quaterna;
	VerifyTrue((num_quaterna_used_target % portion_num_quaterna) == 0);
	
	VerifyTrue(num_nodes_used == num_quaterna_used * 4);
	
	VerifyObject(point_buffer);
	
	/*for (Quaterna const * q = quaterna; q < quaterna_used_end; ++ q) 
	{
		VerifyUsed(* q);
	}*/
	
	// Rarely cause a problem and they are often the majority.
	/*for (Quaterna const * q = quaterna_used_end; q < quaterna_end; ++ q) 
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
	
	Node const * parent = q.nodes[0].GetParent();
	
	VerifyTrue(parent != nullptr);
	VerifyTrue(parent->score == q.parent_score);
	VerifyTrue(parent->score > 0);
	
	for (int i = 0; i < 4; ++ i)
	{
		form::Node const & sibling = q.nodes[i];
		
		// All four siblings should have the same parent.
		VerifyTrue(q.nodes[i].GetParent() == parent);
		Node const * children = sibling.GetChildren();
		if (children != nullptr)
		{
			VerifyArrayElement(children, nodes, nodes_used_end);
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
		
		VerifyTrue(sibling.GetParent() == nullptr);
		VerifyTrue(! sibling.HasChildren());
		VerifyTrue(sibling.score == 0);
	}
	
	VerifyObject(q);
}
#endif

#if defined(DUMP)
DUMP_OPERATOR_DEFINITION(form, NodeBuffer)
{
	for (Quaterna * r = rhs.quaterna; r != rhs.quaterna_used_end; ++ r) {
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

float form::NodeBuffer::GetMinParentScore() const
{
	if (quaterna_used_end > quaterna)
	{
		Quaterna last_quaterna = quaterna_used_end [-1];
		return last_quaterna.parent_score;
	}
	else
	{
		return 0;
	}
}

void form::NodeBuffer::SetNumQuaternaUsedTarget(int n)
{
	if (fix_num_quaterna != 0)
	{
		return;
	}

	Quaterna * new_quaterna_used_end = quaterna + n;
	VerifyArrayElement(new_quaterna_used_end, quaterna, quaterna_end + 1);
	
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
	
	VerifyObject(* this);
}

// TODO: Are these needed now?
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
	VerifyObject (* this);

	node_score_functor.SetCameraRay(new_camera_ray);

	// Is the new camera ray significantly different to 
	// the one used to last score the bulk of the node buffer?
	if (node_score_functor.IsSignificantlyDifferent(cached_node_score_ray))
	{
		UpdateNodeScores();
		cached_node_score_ray = new_camera_ray;
	}
	
	UpdateNodes();
	
	VerifyObject(* this);
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
	VerifyObject(* this);
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
		UpdateQuaterna();
		
		// Finally, using the quaterna,
		// replace nodes whose parent's scores have dropped enough
		// with ones whose score have increased enough.
		if (! ChurnNodes())
		{
			break;
		}
	}
	while (IsNodeChurnIntensive());
}

void form::NodeBuffer::UpdateNodeScores()
{
	// TODO: Try ForEachQuaterna. Faster?
	ForEachNode<CalculateNodeScoreFunctor &>(node_score_functor, 1024, true);
}

void form::NodeBuffer::UpdateQuaterna()
{
	// Reflect the new scores in the quaterna.
	UpdateQuaternaScores();
	
	// Now resort the quaterna so they are in order again.
	SortQuaterna();
}

void form::NodeBuffer::UpdateQuaternaScores()
{
	ForEachQuaterna(UpdateQuaternaScore, 1024, true);
	
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
	ExpandNodeFunctor f(* this);

	ForEachQuaterna<ExpandNodeFunctor &>(f, 1);
	//ForEachNode<ExpandNodeFunctor &> (f, 1);
	
	return f.GetNumExpanded() > 0;
}

void form::NodeBuffer::GenerateMesh(Mesh & mesh) 
{
	VerifyObject(* this);
	
	point_buffer.Clear();
	mesh.Clear();

	GenerateMeshFunctor mesh_functor(mesh);

	ForEachNode<GenerateMeshFunctor &>(mesh_functor, 1024, true);
}

///////////////////////////////////////////////////////
// Node-related members.

float form::NodeBuffer::GetWorseReplacableQuaternaScore() const
{
	// See if there are unused quaterna.
	if (quaterna_used_end != quaterna_used_end_target)
	{
		// As any used quaterna could replace an unused quaterna,
		// we ought to aim pretty low. 
		return -1;
	}
	
	// Find the (known) lowest-scoring quaterna in used.
	if (quaterna_sorted_end > quaterna) 
	{
		// Ok, lets try the end of the sequence of sorted quaterna...
		Quaterna & reusable_quaterna = quaterna_sorted_end [- 1];
		return reusable_quaterna.parent_score;
	}
	
	// We're clean out of nodes!
	// Unless the node tree is somehow eating itself,
	// this probably means a root nodes is expanding
	// and there are no nodes. 
	return std::numeric_limits<float>::max();
}

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
		for (Node * ancestor = & node; ancestor != nullptr; ancestor = ancestor->GetParent())
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

	node.SetChildren(worst_children);
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

void form::NodeBuffer::CollapseNodes(Node & root)
{
	// Remove the entire tree from the given root.
	CollapseNode(root);
	
	// Run an update to restore the state of the buffer ...
	UpdateQuaterna();
	
	// ... except there may still be unused nodes in the used area.
	Quaterna * old_quaterna_used_end = quaterna_used_end;

	// Roll back to the start of the unused quaterna.
	do
		-- quaterna_used_end;
	while (quaterna_used_end >= quaterna && ! quaterna_used_end->IsInUse());
	++ quaterna_used_end;
	
	if (quaterna_sorted_end > quaterna_used_end)
	{
		quaterna_sorted_end = quaterna_used_end;
	}
	
	// Swap used nodes in unused range with unused nodes in used range.
	FixUpDecreasedNodes(old_quaterna_used_end);
}

void form::NodeBuffer::CollapseNode(Node & node)
{
	Node * children = node.GetChildren();
	if (children != nullptr) {
		DeinitChildren(children);
		node.SetChildren(nullptr);
	}
}

// Make sure cousins and their mid-point_buffer match up.
// Also make sure all children know who their parent is.
void form::NodeBuffer::InitChildPointers(Node & parent_node)
{
	Node::Triplet const * parent_triple = parent_node.triple;
	Node * child_nodes = parent_node.GetChildren();
	Node & center = child_nodes[3];
	
	for (int i = 0; i < 3; ++ i)
	{
		Node & node = child_nodes[i];

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
			Node * parent_cousin_children_j = parent_cousin_j->GetChildren();
			if (parent_cousin_children_j != nullptr) {
				node.SetCousin(j, parent_cousin_children_j[k]);
			}
		}
		
		// dextrosinister #2
		Assert(node.triple[k].corner == parent_triple[j].mid_point);
		Node * parent_cousin_k = parent_triple[k].cousin;
		if (parent_cousin_k != nullptr) {
			Node * parent_cousin_children_k = parent_cousin_k->GetChildren();
			if (parent_cousin_children_k != nullptr) {
				node.SetCousin(k, parent_cousin_children_k[j]);
			}
		}
		
		node.seed = parent_node.GetChildSeed(i);
		node.SetParent(& parent_node);
	}
	
	center.seed = parent_node.GetChildSeed(3);
	center.SetParent(& parent_node);
}

void form::NodeBuffer::DeinitChildren(Node * children)
{
	Assert(children->GetParent()->GetChildren() == children);
	children->GetParent()->SetChildren(nullptr);
	
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
	
	node.SetParent(nullptr);
	node.score = 0;
}

void form::NodeBuffer::SubstituteChildren(Node * substitute, Node * original)
{
	Node * parent = original->GetParent();
	
	if (parent != nullptr) 
	{
		Assert(parent->GetChildren() == original);
		parent->SetChildren(substitute);
		
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
	Node * node_it = child.GetChildren();
	if (node_it != nullptr) {
		Node * node_end = node_it + 4;
		do {
			Assert(node_it->GetParent() != & child);
			node_it->SetParent(& child);
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
	
	VerifyObject(* this);
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
	
	VerifyObject (* this);
}

void form::NodeBuffer::DecreaseQuaterna(Quaterna * new_quaterna_used_end)
{
	// Verify that the input is indeed a decrease.
	VerifyArrayElement(new_quaterna_used_end, quaterna, quaterna_used_end + 1);
	
	// Loop through used quats backwards from far end.
	do 
	{
		Quaterna & q = quaterna_used_end [- 1];
		Node * quaterna_nodes = q.nodes;
		
		// Is the quaterna is being used?
		if (q.IsInUse()) 
		{
			// Is it a leaf, i.e. it is the quaterna associated with a grandparent?
			if (q.HasGrandChildren()) 
			{
				// If so, we cannot easily remove this quaterna.
				// It's best to stop the reduction at this element.
				break;
			}
			
			DeinitChildren(quaterna_nodes);
			
			// It is no longer being used.
			q.parent_score = -1;
		}

		// Either way, there should be no children.
		Assert(! quaterna_nodes[0].HasChildren());
		Assert(! quaterna_nodes[1].HasChildren());
		Assert(! quaterna_nodes[2].HasChildren());
		Assert(! quaterna_nodes[3].HasChildren());
		
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

#define WIP 0
void form::NodeBuffer::FixUpDecreasedNodes(Quaterna * old_quaterna_used_end)
{
	// From the new used quaterna value, figure out new used node value.
#if (WIP == 0)
	int new_num_quaterna_used = quaterna_used_end - quaterna;
	int new_num_nodes_used = new_num_quaterna_used << 2;	// 4 nodes per quaterna
	nodes_used_end = nodes + new_num_nodes_used;
#endif
	VerifyArrayElement(nodes_used_end, nodes, nodes_end + 1);
	
	// Use this pointer to walk down the used quaterna array.
	Quaterna * used_quaterna = quaterna_used_end;
	
	// For all the quaterna that have been freed up by the reduction...
	for (Quaterna * unused_quaterna = old_quaterna_used_end - 1; unused_quaterna >= quaterna_used_end; -- unused_quaterna)
	{
		Assert(! unused_quaterna->IsInUse());
		
		Node * unused_nodes = unused_quaterna->nodes;
		
		// Is a quatern past the end of used quaterna
		// pointing to a node before the end of used nodes?
		if (unused_nodes < nodes_used_end)
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

			SubstituteChildren(unused_nodes, substitute_nodes);
			std::swap(used_quaterna->nodes, unused_quaterna->nodes);
		}
		
#if (WIP == 1)
		// Finally steam-roll over whatever the nodes pointer was because 
		// we want unused quaterna to point to their equivalent in the node array;
		nodes_used_end -= 4;
		unused_quaterna->nodes = nodes_used_end;
#endif
	}

	// Finally, the nodes were swapped between used and unused quats in any old order.
	// But the unused nodes need to line up with the unused quaterna.
	std::sort(quaterna_used_end, old_quaterna_used_end, QuaternaSortUnused);
	
	VerifyObject(* this);
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachNode(FUNCTOR f, size_t step_size, bool parallel)
{
	if (nodes_used_end == nodes)
	{
		return;
	}
	
	if (! parallel && step_size == 1)
	{
		core::for_each <form::Node *, FUNCTOR> (nodes, nodes_used_end, f);
	}
	else 
	{
		core::for_each<form::Node *, FUNCTOR, portion_num_nodes>(nodes, nodes_used_end, step_size, f, parallel);
	}
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachQuaterna(FUNCTOR f, size_t step_size, bool parallel)
{
	if (quaterna_used_end == quaterna)
	{
		return;
	}

	if (! parallel && step_size == 1)
	{
		core::for_each<form::Quaterna *, FUNCTOR>(quaterna, quaterna_used_end, f);
	}
	else
	{
		core::for_each<form::Quaterna *, FUNCTOR, portion_num_quaterna>(quaterna, quaterna_used_end, step_size, f, parallel);
	}
}
