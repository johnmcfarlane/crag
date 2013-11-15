//
//  Surrounding.cpp
//  crag
//
//  Created by John on 3013-11-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Surrounding.h"

#include "ExpandNodeFunctor.h"
#include "GatherExpandableNodesFunctor.h"
#include "GenerateMeshFunctor.h"
#include "Polyhedron.h"

#include "core/ConfigEntry.h"

using namespace form;

CONFIG_DECLARE (profile_mode, bool);

////////////////////////////////////////////////////////////////////////////////
// file-local definitions

namespace
{
	// If profile_mode==true, this number of quaterne is enforced always (with reason).
	// It is useful for eliminating the adaptive quaterne count algorithm during debugging.
	CONFIG_DEFINE (profile_num_quaterne, int, 6400);

	bool QuaternaSortUnused(Quaterna const & lhs, Quaterna const & rhs)
	{
		return lhs.nodes < rhs.nodes;
	}

	// Repair the pointers that point TO this node.
	void RepairChild(Node & child)
	{
		// Repair children's parent pointers.
		Node * node_it = child.GetChildren();
		if (node_it != nullptr) {
			Node * node_end = node_it + 4;
			do {
				ASSERT(node_it->GetParent() != & child);
				node_it->SetParent(& child);
				++ node_it;
			}	while (node_it < node_end);
		}

		// Repair cousin pointers.
		Node::Triplet * triple = child.triple;
		for (int i = 0; i < 3; ++ i) {
			Node * cousin = triple[i].cousin;
			if (cousin != nullptr) {
				ASSERT(cousin->triple[i].cousin != nullptr);
				ASSERT(cousin->triple[i].cousin != & child);
				cousin->triple[i].cousin = & child;
			}
		}
	}

	void SubstituteChildren(Node * substitute, Node * original)
	{
		Node * parent = original->GetParent();
	
		if (parent != nullptr) 
		{
			ASSERT(parent->GetChildren() == original);
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

	void FixUpDecreasedNodes(Quaterna * begin, Quaterna * end, int old_num_quaterne, Node const & new_nodes_used_end)
	{
		Quaterna * old_quaterne_used_end = begin + old_num_quaterne;
		ASSERT(old_quaterne_used_end > end);
	
		// Use this pointer to walk down the used quaterne array.
		Quaterna * used_quaterna = end;
	
		// For all the quaterne that have been freed up by the reduction...
		for (Quaterna * unused_quaterna = old_quaterne_used_end - 1; unused_quaterna >= end; -- unused_quaterna)
		{
			ASSERT(! unused_quaterna->IsInUse());
		
			Node * unused_nodes = unused_quaterna->nodes;
		
			// Is a quatern past the end of used quaterne
			// pointing to a node before the end of used nodes?
			if (unused_nodes < & new_nodes_used_end)
			{
				// It needs to point to a quad of nodes past the end of used nodes.
				// There must be a corresponding quad of nodes in such a position that is in use.
				// They must be swapped.
			
				// Find the used quaterne whose nodes are past the end of the used range.
				Node * substitute_nodes;
				do
				{
					-- used_quaterna;
					ASSERT(used_quaterna >= begin);
					substitute_nodes = used_quaterna->nodes;
				}
				while (substitute_nodes < & new_nodes_used_end);

				SubstituteChildren(unused_nodes, substitute_nodes);
				std::swap(used_quaterna->nodes, unused_quaterna->nodes);
			}
		
			//// TODO: Finally steam-roll over whatever the nodes pointer was because 
			//// we want unused quaterne to point to their equivalent in the node array;
			//nodes_used_end -= 4;
			//unused_quaterna->nodes = nodes_used_end;
		}

		// Finally, the nodes were swapped between used and unused quats in any old order.
		// But the unused nodes need to line up with the unused quaterne.
		std::sort(end, old_quaterne_used_end, QuaternaSortUnused);
	}
}


////////////////////////////////////////////////////////////////////////////////
// Surrounding functions

Surrounding::Surrounding(size_t max_num_quaterne)
: _node_buffer(max_num_quaterne * num_nodes_per_quaterna)
, _quaterna_buffer(max_num_quaterne)
, _target_num_quaterne(std::min(profile_mode ? profile_num_quaterne : 0, static_cast<int>(max_num_quaterne)))
, point_buffer(max_num_quaterne * num_verts_per_quaterna)
, cached_node_score_ray(CalculateNodeScoreFunctor::GetInvalidRay())
, _expandable_nodes(max_num_quaterne * num_nodes_per_quaterna)
{
	InitQuaterna(std::begin(_quaterna_buffer) + _quaterna_buffer.capacity());

	VerifyObject(* this);
}

Surrounding::~Surrounding()
{
	VerifyObject(* this);
}

#if defined(VERIFY)
void Surrounding::Verify() const
{
	VerifyObject(_node_buffer);
	VerifyObject(_quaterna_buffer);

	VerifyOp(_target_num_quaterne, <=, _quaterna_buffer.capacity());
	VerifyOp(_target_num_quaterne, >=, _quaterna_buffer.size());
	
	auto num_nodes_used = GetNumNodesUsed();
	VerifyTrue((num_nodes_used % num_nodes_per_quaterna) == 0);
	
	auto num_quaterne_used = GetNumQuaternaUsed();
	
	VerifyTrue(num_nodes_used == num_quaterne_used * 4);
	
	VerifyObject(point_buffer);
	
	/*for (Quaterna const * q = quaterne; q < quaterne_used_end; ++ q) 
	{
		VerifyUsed(* q);
	}*/
	
	// Rarely cause a problem and they are often the majority.
	/*for (Quaterna const * q = quaterne_used_end; q < quaterne_end; ++ q) 
	{
		VerifyUnused(* q);
	}
	
	for (Quaterna const * q2 = quaterne + 1; q2 < quaterne_sorted_end; ++ q2) 
	{
		Quaterna const * q1 = q2 - 1;
		VerifyTrue(q2->parent_score <= q1->parent_score);
	}*/
}

void Surrounding::VerifyUsed(Quaterna const & q) const
{
	Node const & n = ref(q.nodes);
	_node_buffer.VerifyUsed(n);
	
	Node const * parent = q.nodes[0].GetParent();
	
	VerifyTrue(parent != nullptr);
	VerifyTrue(parent->score == q.parent_score);
	VerifyTrue(parent->score > 0);
	
	for (int i = 0; i < 4; ++ i)
	{
		Node const & sibling = q.nodes[i];
		
		// All four siblings should have the same parent.
		VerifyTrue(q.nodes[i].GetParent() == parent);
		Node const * children = sibling.GetChildren();
		if (children != nullptr)
		{
			_node_buffer.VerifyUsed(* children);
		}

		for (int j = 0; j < 3; ++ j)
		{
			Point const & point = sibling.GetCorner(j);
			VerifyObjectRef(point);
			point_buffer.VerifyAllocatedElement(point);
		}
	}
	
	VerifyObject(q);
}

void Surrounding::VerifyUnused(Quaterna const & q) const
{
	VerifyTrue(q.parent_score == -1);
	
	Node const * n = q.nodes;
	VerifyTrue(& _node_buffer[(& q - std::begin(_quaterna_buffer)) * 4] == n);
	
	for (int i = 0; i < 4; ++ i)
	{
		Node const & sibling = q.nodes[i];
		
		VerifyTrue(sibling.GetParent() == nullptr);
		VerifyTrue(! sibling.HasChildren());
		VerifyTrue(sibling.score == 0);
	}
	
	VerifyObject(q);
}
#endif

int Surrounding::GetNumNodesUsed() const
{
	return static_cast<std::size_t>(_node_buffer.GetSize());
}

int Surrounding::GetNumQuaternaUsed() const
{
	return _quaterna_buffer.size();
}

float Surrounding::GetMinParentScore() const
{
	if (_quaterna_buffer.empty())
	{
		return 0;
	}

	auto & last_quaterna = _quaterna_buffer.back();
	return last_quaterna.parent_score;
}

Scalar Surrounding::GetMinLeafDistanceSquared()
{
	return node_score_functor.GetMinLeafDistanceSquared();
}

int Surrounding::GetTargetNumQuaterna() const
{
	return _target_num_quaterne;
}

void Surrounding::SetTargetNumQuaterna(int target_num_quaterne)
{
	VerifyOp(target_num_quaterne, <=, _quaterna_buffer.capacity());

	if (profile_mode)
	{
		return;
	}

	int num_quaterna_used = _quaterna_buffer.size();
	std::ptrdiff_t growth = target_num_quaterne - num_quaterna_used;
	
	if (growth > 0)
	{
		IncreaseNodes(target_num_quaterne);
	}
	else if (growth < 0)
	{
		DecreaseNodes(target_num_quaterne);
	}
}

// This is the main tick function for all things 'nodey'.
// It is also where a considerable amount of the SceneThread's time is spent.
void Surrounding::Tick(Ray3 const & new_camera_ray)
{
	VerifyObjectRef(new_camera_ray);
	VerifyObject (* this);

	node_score_functor.SetCameraRay(new_camera_ray);

	// Is the new camera ray significantly different to 
	// the one used to last score the bulk of the node buffer?
	// TODO: Disabled. Causes complications, not least fluctuations in the frame-rate
	// which in turn cause fluctuations in the node count.
	if (node_score_functor.IsSignificantlyDifferent(cached_node_score_ray) || true)
	{
		UpdateNodeScores();
		cached_node_score_ray = new_camera_ray;
	}
	
	UpdateNodes();
	
	VerifyObject(* this);
}

void Surrounding::OnReset()
{
	ASSERT(point_buffer.IsEmpty());
	InitQuaterna(std::end(_quaterna_buffer));

	_node_buffer.Clear();
	
	_quaterna_buffer.Clear();

	// Half the target number of nodes.
	// Probably not a smart idea.
	//quaterne_used_end_target -= (quaterne_used_end_target - quaterne) >> 1;
	VerifyObject(* this);
}

void Surrounding::ResetNodeOrigins(Vector3 const & origin_delta)
{
	_node_buffer.ResetNodeOrigins(origin_delta);
}

void Surrounding::InitQuaterna(Quaterna const * end)
{
	auto n = std::begin(_node_buffer);
	for (Quaterna * iterator = std::begin(_quaterna_buffer); iterator < end; n += 4, ++ iterator)
	{
		iterator->parent_score = -1;
		iterator->nodes = n;
	}
}

void Surrounding::UpdateNodes()
{
	do 
	{
		UpdateQuaterna();
		
		// Finally, using the quaterne,
		// replace nodes whose parent's scores have dropped enough
		// with ones whose score have increased enough.
		if (! ChurnNodes())
		{
			break;
		}
	}
	while (IsNodeChurnIntensive());
}

void Surrounding::UpdateNodeScores()
{
	node_score_functor.ResetCounters();
	_node_buffer.ForEach<CalculateNodeScoreFunctor &>(node_score_functor, 1024, true);
}

void Surrounding::UpdateQuaterna()
{
	// Reflect the new scores in the quaterne.
	_quaterna_buffer.UpdateScores();
	
	// Now resort the quaterne so they are in order again.
	_quaterna_buffer.Sort();
}

bool Surrounding::ChurnNodes()
{
	ASSERT(_expandable_nodes.size() == 0);
	
	// Populate vector with nodes which might want expanding.
	GatherExpandableNodesFunctor gather_functor(* this, _expandable_nodes);
	_quaterna_buffer.ForEach<GatherExpandableNodesFunctor &>(gather_functor);

	// Traverse the vector and try and expand the nodes.
	ExpandNodeFunctor expand_functor(* this);
	core::for_each <SmpNodeVector::iterator, ExpandNodeFunctor &>(_expandable_nodes.begin(), _expandable_nodes.end(), expand_functor);
	
	_expandable_nodes.clear();
	
	return expand_functor.GetNumExpanded() > 0;
}

void Surrounding::GenerateMesh(Mesh & mesh) 
{
	VerifyObject(* this);
	
	point_buffer.ClearPointers();
	mesh.Clear();

	GenerateMeshFunctor mesh_functor(node_score_functor.GetLeafScoreRange(), mesh);

	_node_buffer.ForEach<GenerateMeshFunctor &>(mesh_functor, 1024, true);
}

///////////////////////////////////////////////////////
// Node-related members.

float Surrounding::GetLowestSortedQuaternaScore() const
{
	// See if there are unused quaterne available.
	if (_target_num_quaterne != _quaterna_buffer.size())
	{
		// As any used quaterna could replace an unused quaterna,
		// we ought to aim pretty low. 
		return std::numeric_limits<float>::min();
	}
	
	return _quaterna_buffer.GetLowestSortedScore();
}

bool Surrounding::ExpandNode(Node & node) 
{
	ASSERT(node.IsExpandable());

	// Try and get an unused quaterna.
	if (_quaterna_buffer.size() != _target_num_quaterne)
	{
		// We currently wish to expand the number of nodes/quaterne used,
		// so attempt to use the next unused quaterna in the array.
		auto & unused_quaterna = _quaterna_buffer.Grow();

		if (! ExpandNode(node, unused_quaterna))
		{
			_quaterna_buffer.Shrink();
			return false;
		}
		
		_node_buffer.Push(4);
		return true;
	}
	
	// Find the lowest-scoring quaterna in used and attempt to reuse it.
	auto reusable_quaterna = _quaterna_buffer.GetLowestSorted();
	if (! reusable_quaterna) 
	{
		// We're out of sorted nodes.
		return false;
	}
	
	// Get the score to use to find the 'worst' Quaterna.
	// We don't want to nuke a quaterna with a worse one, hence using the node score.
	// But also, there's a slim chance that node's parent has a worse score than node's
	// and we can't end up finding a quaterna that includes node as node's new children
	// because that would cause a time paradox in the fabric of space.
	float score = node.score;
	
	// Ok, lets try the end of the sequence of sorted quaterne...
	
	if (! reusable_quaterna->IsSuitableReplacement(score)) 
	{
		return false;
	}

	// Make sure that the node isn't replacing itself or one of its ancestors.
	for (Node * ancestor = & node; ancestor != nullptr; ancestor = ancestor->GetParent())
	{
		if (ancestor >= reusable_quaterna->nodes && ancestor < reusable_quaterna->nodes + 4)
		{
			return false;
		}
	}
	
	if (! ExpandNode(node, * reusable_quaterna))
	{
		return false;
	}
	
	_quaterna_buffer.DecrementSorted();

	return true;
}

bool Surrounding::ExpandNode(Node & node, Quaterna & children_quaterna)	
{
	Node * worst_children = children_quaterna.nodes;
	ASSERT(worst_children + 0 != & node);
	ASSERT(worst_children + 1 != & node);
	ASSERT(worst_children + 2 != & node);
	ASSERT(worst_children + 3 != & node);
	
	// Note that after this point, expansion may fail but the node may have new mid-points.
	Polyhedron & polyhedron = GetPolyhedron(node);
	if (! node.InitMidPoints(polyhedron, point_buffer))
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
	
	// Deinit children.
	bool is_in_use = children_quaterna.IsInUse();
	if (is_in_use) 
	{
		DeinitChildren(worst_children);
	}
	ASSERT(worst_children[0].score == 0);
	ASSERT(worst_children[1].score == 0);
	ASSERT(worst_children[2].score == 0);
	ASSERT(worst_children[3].score == 0);
	
	worst_children[0] = children_copy[0];
	worst_children[1] = children_copy[1];
	worst_children[2] = children_copy[2];
	worst_children[3] = children_copy[3];

	node.SetChildren(worst_children);
	InitChildPointers(node);
	
	children_quaterna.parent_score = node.score;
	
	// If Surrounding::Tick iterates over the nodes multiple times, the new-comers need their scores corrected.
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

void Surrounding::CollapseNodes(Node & root)
{
	// Remove the entire tree from the given root.
	CollapseNode(root);
	
	// Run an update to restore the state of the buffer ...
	UpdateQuaterna();
	
	// ... except there may still be unused nodes in the used area.
	auto old_num_quaterne = _quaterna_buffer.size();

	// Roll back to the end of the used quaterne.
	while (! _quaterna_buffer.empty() && ! _quaterna_buffer.back().IsInUse())
	{
		_quaterna_buffer.Shrink();
	}
	
	// Swap used nodes in unused range with unused nodes in used range.
	FixUpDecreasedNodes(old_num_quaterne);
}

void Surrounding::CollapseNode(Node & node)
{
	Node * children = node.GetChildren();
	if (children != nullptr)
	{
		DeinitChildren(children);
		node.SetChildren(nullptr);
	}
}

// Make sure cousins and their mid-point_buffer match up.
// Also make sure all children know who their parent is.
void Surrounding::InitChildPointers(Node & parent_node)
{
	Node::Triplet const * parent_triple = parent_node.triple;
	Node * child_nodes = parent_node.GetChildren();
	Node & center = child_nodes[3];
	
	for (int i = 0; i < 3; ++ i)
	{
		Node & node = child_nodes[i];

		// This is the side which the child shares with its sibling in the center.
		// anteroposterior
		ASSERT(node.triple[i].corner == parent_triple[i].corner);
		ASSERT(node.triple[i].mid_point == nullptr);
		node.SetCousin(i, center);
		
		int j = TriMod(i + 1);
		int k = TriMod(i + 2);
		
		// The other two corners are shared with children of the parent's children.

		// dextrosinister #1
		ASSERT(node.triple[j].corner == parent_triple[k].mid_point);
		Node * parent_cousin_j = parent_triple[j].cousin;
		if (parent_cousin_j != nullptr) {
			Node * parent_cousin_children_j = parent_cousin_j->GetChildren();
			if (parent_cousin_children_j != nullptr) {
				node.SetCousin(j, parent_cousin_children_j[k]);
			}
		}
		
		// dextrosinister #2
		ASSERT(node.triple[k].corner == parent_triple[j].mid_point);
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

void Surrounding::DeinitChildren(Node * children)
{
	ASSERT(children->GetParent()->GetChildren() == children);
	children->GetParent()->SetChildren(nullptr);
	
	DeinitNode(children[0]);
	DeinitNode(children[1]);
	DeinitNode(children[2]);
	DeinitNode(children[3]);
}

// Nulls all the relevant pointers, disconnects cousins, frees verts.
void Surrounding::DeinitNode(Node & node)
{
	CollapseNode(node);
	
	for (int i = 0; i < 3; ++ i)
	{
		Node::Triplet & t = node.triple[i];
		
		t.corner = nullptr;
		
		if (t.cousin != nullptr)
		{
			Node::Triplet & mirror = t.cousin->triple[i];
			ASSERT(mirror.cousin == & node);
			ASSERT(mirror.mid_point == t.mid_point);
			
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
				point_buffer.Destroy(t.mid_point);
				t.mid_point = nullptr;
			}
		}
		
		ASSERT(t.mid_point == nullptr);
	}
	
	node.SetParent(nullptr);
	node.score = 0;
}

void Surrounding::IncreaseNodes(int target_num_quaterne)
{
	// Verify that the input is indeed a decrease.
	VerifyOp(target_num_quaterne, >, _quaterna_buffer.size());
	VerifyOp(target_num_quaterne, <=, _quaterna_buffer.capacity());
	
	// Increasing the target number of nodes is simply a matter of setting a value. 
	// The target pointer now point_buffer into the range of unused quaterne at the end of the array.
	_target_num_quaterne = target_num_quaterne;
	
	VerifyObject(* this);
}

// Reduce the number of used nodes and, accordingly, the number of used quaterne.
// This is quite an involved and painful process which sometimes fails half-way through.
void Surrounding::DecreaseNodes(int target_num_quaterne)
{
	auto old_num_quaterne = _quaterna_buffer.size();

	// First decrease the number of quaterne. This is the bit that sometimes fails.
	DecreaseQuaterna(target_num_quaterne);
	
	// Was there any decrease at all?
	if (old_num_quaterne == _quaterna_buffer.size())
	{
		ASSERT(false);	// This isn't deadly fatal but serious enough that I'd like to know it happens.
		return;
	}
	
	// Because the nodes aren't in the correct order, decreasing them is somewhat more tricky.
	FixUpDecreasedNodes(old_num_quaterne);
	
	VerifyObject (* this);
}

void Surrounding::DecreaseQuaterna(int new_num_quaterne)
{
	// Verify that the input is indeed a decrease.
	VerifyOp(new_num_quaterne, <, _quaterna_buffer.size());
	
	// Loop through used quats backwards from far end.
	do 
	{
		Quaterna & q = _quaterna_buffer.back();
		Node * quaterna_nodes = q.nodes;
		
		// Is the quaterna being used?
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
		ASSERT(! quaterna_nodes[0].HasChildren());
		ASSERT(! quaterna_nodes[1].HasChildren());
		ASSERT(! quaterna_nodes[2].HasChildren());
		ASSERT(! quaterna_nodes[3].HasChildren());
		
		_quaterna_buffer.Shrink();
	}
	while (_quaterna_buffer.size() > new_num_quaterne);
	
	// Target is really for increasing the target during churn.
	_target_num_quaterne = new_num_quaterne;
	VerifyEqual(_target_num_quaterne, _quaterna_buffer.size());
	
	VerifyObject(* this);
}

void Surrounding::FixUpDecreasedNodes(int old_num_quaterne)
{
	auto new_num_quaterne = _quaterna_buffer.size();
	ASSERT(old_num_quaterne > new_num_quaterne);
	
	// From the new used quaterna value, figure out new used node value.
	auto new_num_nodes_used = new_num_quaterne << 2;	// 4 nodes per quaterna
	auto new_nodes_used_end = std::begin(_node_buffer) + new_num_nodes_used;
	auto old_nodes_used_end = std::end(_node_buffer);
	ASSERT(old_nodes_used_end > new_nodes_used_end);

	VerifyArrayElement(new_nodes_used_end, std::begin(_node_buffer), old_nodes_used_end);
	
	::FixUpDecreasedNodes(std::begin(_quaterna_buffer), std::end(_quaterna_buffer), old_num_quaterne, * new_nodes_used_end);
	
	// revise down the number of nodes in use
	_node_buffer.Pop(old_nodes_used_end - new_nodes_used_end);
	ASSERT(new_nodes_used_end == std::end(_node_buffer));
	
	VerifyObject(* this);
}
