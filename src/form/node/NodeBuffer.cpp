//
//  NodeBuffer.cpp
//  crag
//
//  Created by John on 2/17/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "NodeBuffer.h"

#include "ExpandNodeFunctor.h"
#include "GatherExpandableNodesFunctor.h"
#include "GenerateMeshFunctor.h"
#include "Quaterna.h"
#include "Shader.h"

#include "form/scene/Polyhedron.h"

#include "smp/for_each.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


CONFIG_DECLARE (profile_mode, bool);


namespace 
{

	// If profile_mode==true, this number of quaterne is enforced always (with reason).
	// It is useful for eliminating the adaptive quaterne count algorithm during debugging.
	CONFIG_DEFINE (profile_num_quaterne, int, 6400);
	
	
	////////////////////////////////////////////////////////////////////////////////
	// local function definitions
	
	inline void UpdateQuaternaScore(form::Quaterna & q) 
	{
		form::Node * parent = q.nodes[0].GetParent();
		q.parent_score = (parent != nullptr) ? parent->score : -1;
	}
	
	bool QuaternaSortUnused(form::Quaterna const & lhs, form::Quaterna const & rhs)
	{
		return lhs.nodes < rhs.nodes;
	}

}


////////////////////////////////////////////////////////////////////////////////
// NodeBuffer functions

form::NodeBuffer::NodeBuffer(size_t max_num_quaterne)
: nodes(reinterpret_cast<Node *>(Allocate(sizeof(Node) * max_num_quaterne * num_nodes_per_quaterna, 128)))
, nodes_used_end(nodes)
, nodes_end(nodes + max_num_quaterne * num_nodes_per_quaterna)
, quaterne(new Quaterna [max_num_quaterne])
, quaterne_sorted_end(quaterne)
, quaterne_used_end(quaterne)
, quaterne_used_end_target(quaterne + std::min(profile_mode ? profile_num_quaterne : 0, static_cast<int>(max_num_quaterne)))
, quaterne_end(quaterne + max_num_quaterne)
, point_buffer(max_num_quaterne * num_verts_per_quaterna)
, cached_node_score_ray(CalculateNodeScoreFunctor::GetInvalidRay())
, _expandable_nodes(max_num_quaterne * num_nodes_per_quaterna)
{
	ZeroArray(nodes, max_num_quaterne * num_nodes_per_quaterna);

	InitQuaterna(quaterne_end);

	VerifyObject(* this);
}

form::NodeBuffer::~NodeBuffer()
{
	VerifyObject(* this);

#if ! defined(NDEBUG)
	for (Quaterna const * i = quaterne; i != quaterne_used_end; ++ i)
	{
		ASSERT(! i->HasGrandChildren());
		ASSERT(! i->nodes[0].IsInUse());
		ASSERT(! i->nodes[1].IsInUse());
		ASSERT(! i->nodes[2].IsInUse());
		ASSERT(! i->nodes[3].IsInUse());
	}
#endif
	
	delete quaterne;

	//delete nodes;
	Free(nodes);
}

#if defined(VERIFY)
void form::NodeBuffer::Verify() const
{
	VerifyArrayPointer(nodes_used_end, nodes, nodes_end);
	
	VerifyArrayPointer(quaterne_sorted_end, quaterne);
	VerifyArrayPointer(quaterne_used_end, quaterne);
	VerifyArrayPointer(quaterne_used_end_target, quaterne);
	VerifyArrayPointer(quaterne_end, quaterne);
	
	VerifyTrue(quaterne_sorted_end >= quaterne);
	VerifyTrue(quaterne_used_end >= quaterne_sorted_end);
	VerifyTrue(quaterne_used_end_target >= quaterne_used_end);
	VerifyTrue(quaterne_end >= quaterne_used_end_target);

	auto num_nodes_used = nodes_used_end - nodes;
	VerifyTrue((num_nodes_used % num_nodes_per_quaterna) == 0);
	
	auto num_quaterne_used = quaterne_used_end - quaterne;
	
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

		for (int j = 0; j < 3; ++ j)
		{
			Point const & point = sibling.GetCorner(j);
			VerifyObjectRef(point);
			point_buffer.VerifyAllocatedElement(point);
		}
	}
	
	VerifyObject(q);
}

void form::NodeBuffer::VerifyUnused(Quaterna const & q) const
{
	VerifyTrue(q.parent_score == -1);
	
	Node const * n = q.nodes;
	VerifyTrue(n - nodes == (& q - quaterne) * 4);
	
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

std::size_t form::NodeBuffer::GetNumNodesUsed() const
{
	return nodes_used_end - nodes;
}

std::size_t form::NodeBuffer::GetNumQuaternaUsed() const
{
	return quaterne_used_end - quaterne;
}

std::size_t form::NodeBuffer::GetNumQuaternaUsedTarget() const
{
	return quaterne_used_end_target - quaterne;
}

float form::NodeBuffer::GetMinParentScore() const
{
	if (quaterne_used_end > quaterne)
	{
		Quaterna last_quaterna = quaterne_used_end [-1];
		return last_quaterna.parent_score;
	}
	else
	{
		return 0;
	}
}

form::Scalar form::NodeBuffer::GetMinLeafDistanceSquared()
{
	return node_score_functor.GetMinLeafDistanceSquared();
}

void form::NodeBuffer::SetNumQuaternaUsedTarget(std::size_t n)
{
	if (profile_mode)
	{
		return;
	}

	Quaterna * new_quaterne_used_end = quaterne + n;
	VerifyArrayElement(new_quaterne_used_end, quaterne, quaterne_end + 1);
	
	if (new_quaterne_used_end == quaterne_used_end) 
	{
		// No change. Just make sure the target is the same
		quaterne_used_end_target = new_quaterne_used_end;
	}
	else if (new_quaterne_used_end > quaterne_used_end) 
	{
		IncreaseNodes(new_quaterne_used_end);
	}
	else if (new_quaterne_used_end < quaterne_used_end)
	{
		DecreaseNodes(new_quaterne_used_end);
	}
	
	VerifyObject(* this);
}

// This is the main tick function for all things 'nodey'.
// It is also where a considerable amount of the SceneThread's time is spent.
void form::NodeBuffer::Tick(Ray3 const & new_camera_ray)
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

void form::NodeBuffer::OnReset()
{
	ASSERT(point_buffer.IsEmpty());
	InitQuaterna(quaterne_used_end);

	nodes_used_end = nodes;
	quaterne_sorted_end = quaterne_used_end = quaterne;
	
	// Half the target number of nodes.
	// Probably not a smart idea.
	//quaterne_used_end_target -= (quaterne_used_end_target - quaterne) >> 1;
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
	for (Quaterna * iterator = quaterne; iterator < end; n += 4, ++ iterator) {
		iterator->parent_score = -1;
		iterator->nodes = n;
	}
}

void form::NodeBuffer::UpdateNodes()
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

void form::NodeBuffer::UpdateNodeScores()
{
	node_score_functor.ResetCounters();
	ForEachNode<CalculateNodeScoreFunctor &>(node_score_functor, 1024, true);
}

void form::NodeBuffer::UpdateQuaterna()
{
	// Reflect the new scores in the quaterne.
	UpdateQuaternaScores();
	
	// Now resort the quaterne so they are in order again.
	SortQuaterna();
}

void form::NodeBuffer::UpdateQuaternaScores()
{
	ForEachQuaterna(UpdateQuaternaScore, 1024, true);
	
	// This basically says: "as far as I know, none of the quaterne are sorted."
	quaterne_sorted_end = quaterne;
}

// Algorithm to sort nodes array. 
void form::NodeBuffer::SortQuaterna()
{
	if (quaterne_sorted_end == quaterne_used_end)
	{
		VerifyObject(* this);
		return;
	}

#if 0
	typedef geom::Vector<Quaterna *, 2> QuaternaRange;

	QuaternaRange unsorted_range =
	{
		quaterne,
		quaterne_used_end - 1
	};
	
	int c = 1;
	while (unsorted_range.x < unsorted_range.y)
	{
		QuaternaRange next_unsorted_range =
		{
			quaterne_used_end,
			quaterne
		};
		
		for (auto i = unsorted_range.x; i != unsorted_range.y; ++ i)
		{
			if (i[0].parent_score < i[1].parent_score)
			{
				std::swap(i[0], i[1]);
				
				if (i < next_unsorted_range.x)
				{
					next_unsorted_range.x = i - 1;
					
					if (next_unsorted_range.x < quaterne)
					{
						next_unsorted_range.x = quaterne;
					}
				}
				
				if (i > next_unsorted_range.y)
				{
					next_unsorted_range.y = i + 1;
				}
			}
		}
		
		if ((-- c) == 0)
		{
			break;
		}
		
		unsorted_range = next_unsorted_range;
	}
#elif 0
	QuaternaRange unsorted_range =
	{
		quaterne,
		quaterne_used_end - 1
	};
	//int c = 0;
	
	for (auto i = quaterne; i != quaterne_used_end; ++ i)
	{
		//DEBUG_MESSAGE("%d:%f", i - quaterne, i->parent_score);
	}
	
	while (unsorted_range.x < unsorted_range.y)
	{
		QuaternaRange next_unsorted_range = unsorted_range;
		Quaterna * i = unsorted_range.x;
		
		for (; i < unsorted_range.y; ++ i)
		{
			if (i[0].parent_score < i[1].parent_score)
			{
				break;
			}
		}
		next_unsorted_range.x = std::max(i - 1, quaterne);
		
		next_unsorted_range.y = next_unsorted_range.x;
		for (; i < unsorted_range.y; ++ i)
		{
			if (i[0].parent_score < i[1].parent_score)
			{
				std::swap(i[0].parent_score, i[1].parent_score);
				next_unsorted_range.y = i;
			}
		}
		
		if (next_unsorted_range.y + 1 == i)
		{
			const Quaterna * end = quaterne_used_end - 1;
			for (unsorted_range.y = i; unsorted_range.y < end; ++ unsorted_range.y)
			{
				if (unsorted_range.y[0].parent_score > unsorted_range.y[1].parent_score)
				{
					break;
				}
				
				std::swap(unsorted_range.y[0], unsorted_range.y[1]);
			}
		}

//		DEBUG_MESSAGE("%d [%d,%d) -> [%d,%d)", c ++,
//					  unsorted_range.x - quaterne, unsorted_range.y - quaterne,
//					  next_unsorted_range.x - quaterne, next_unsorted_range.y - quaterne);
		
		unsorted_range = next_unsorted_range;
	}
#elif 0
	// Sure and steady ... and slow!
//	std::sort(quaterne_sorted_end, quaterne_used_end, [] (form::Quaterna const & lhs, form::Quaterna const & rhs) {
//		return lhs.parent_score > rhs.parent_score;
//	});

	geom::Vector<Quaterna *, 2> range = { quaterne_sorted_end, quaterne_used_end - 1 };
	int c = 0;
	
	while (range.x < range.y - 1)
	{
		geom::Vector<Quaterna *, 2> next_range =
		{
			quaterne_used_end,
			quaterne_sorted_end
		};
		
		for (size_t pass = 0; pass != 2; ++ pass)
		{
			for (Quaterna * i = range.x + pass; i < range.y; i += 2)
			{
				if (i[0].parent_score < i[1].parent_score)
				{
					std::swap(i[0], i[1]);
					if (i <= next_range.x)
					{
						next_range.x = i - 1;
					}
					if (i >= next_range.y)
					{
						next_range.y = i + 2;
					}
				}
			}
			
			range.x = std::max(next_range.x, quaterne);
			range.y = std::min(next_range.y, quaterne_used_end - 1);
			DEBUG_MESSAGE("%d:%d [%d,%d)", c ++, int(pass), range.x - quaterne, range.y - quaterne);
		}
	}
	
#elif 0
	Quaterna * range_begin = quaterne;
	while (true)
	{
		for (size_t pass = 0; pass != 2; ++ pass)
		{
			Quaterna * range_end = quaterne_sorted_end - 1;
			for (Quaterna * i = range_begin + pass; i < range_end; i += 2)
			{
				if (i[0].parent_score < i[1].parent_score)
				{
					std::swap(i[0], i[1]);
				}
			}
		}
		
		size_t range_size = quaterne_sorted_end - range_begin;
		if (range_size < 128)
		{
			break;
		}
		
		range_begin = range_begin + (range_size >> 1) + 1;
	}
#else
	// Sure and steady ... and slow!
	std::sort(quaterne, quaterne_used_end, [] (form::Quaterna const & lhs, form::Quaterna const & rhs) {
		return lhs.parent_score > rhs.parent_score;
	});
#endif
	
	quaterne_sorted_end = quaterne_used_end;
}

bool form::NodeBuffer::ChurnNodes()
{
	ASSERT(_expandable_nodes.size() == 0);
	
	// Populate vector with nodes which might want expanding.
	GatherExpandableNodesFunctor gather_functor(* this, _expandable_nodes);
	ForEachQuaterna<GatherExpandableNodesFunctor &>(gather_functor);

	// Traverse the vector and try and expand the nodes.
	ExpandNodeFunctor expand_functor(* this);
	core::for_each <SmpNodeVector::iterator, ExpandNodeFunctor &>(_expandable_nodes.begin(), _expandable_nodes.end(), expand_functor);
	
	_expandable_nodes.clear();
	
	return expand_functor.GetNumExpanded() > 0;
}

void form::NodeBuffer::GenerateMesh(Mesh & mesh) 
{
	VerifyObject(* this);
	
	point_buffer.ClearPointers();
	mesh.Clear();

	GenerateMeshFunctor mesh_functor(node_score_functor.GetLeafScoreRange(), mesh);

	ForEachNode<GenerateMeshFunctor &>(mesh_functor, 1024, true);
}

///////////////////////////////////////////////////////
// Node-related members.

float form::NodeBuffer::GetWorseReplacableQuaternaScore() const
{
	// See if there are unused quaterne.
	if (quaterne_used_end != quaterne_used_end_target)
	{
		// As any used quaterna could replace an unused quaterna,
		// we ought to aim pretty low. 
		return std::numeric_limits<float>::min();
	}
	
	// Find the (known) lowest-scoring quaterna in used.
	if (quaterne_sorted_end > quaterne) 
	{
		// Ok, lets try the end of the sequence of sorted quaterne...
		Quaterna & reusable_quaterna = quaterne_sorted_end [- 1];
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
	ASSERT(node.IsExpandable());

	// Try and get an unused quaterna.
	if (quaterne_used_end != quaterne_used_end_target)
	{
		// We currently wish to expand the number of nodes/quaterne used,
		// so attempt to use the next unused quaterna in the array.
		Quaterna & unused_quaterna = * quaterne_used_end;

		if (! ExpandNode(node, unused_quaterna))
		{
			return false;
		}
		
		nodes_used_end += 4;
		++ quaterne_used_end;
		return true;
	}
	
	// Find the lowest-scoring quaterna in used and attempt to reuse it.
	if (quaterne_sorted_end > quaterne) 
	{
		// Get the score to use to find the 'worst' Quaterna.
		// We don't want to nuke a quaterna with a worse one, hence using the node score.
		// But also, there's a slim chance that node's parent has a worse score than node's
		// and we can't end up finding a quaterna that includes node as node's new children
		// because that would cause a time paradox in the fabric of space.
		float score = node.score;
		
		// Ok, lets try the end of the sequence of sorted quaterne...
		Quaterna & reusable_quaterna = quaterne_sorted_end [- 1];			
		
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
		
		-- quaterne_sorted_end;
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
	Quaterna * old_quaterne_used_end = quaterne_used_end;

	// Roll back to the start of the unused quaterne.
	do
		-- quaterne_used_end;
	while (quaterne_used_end >= quaterne && ! quaterne_used_end->IsInUse());
	++ quaterne_used_end;
	
	if (quaterne_sorted_end > quaterne_used_end)
	{
		quaterne_sorted_end = quaterne_used_end;
	}
	
	// Swap used nodes in unused range with unused nodes in used range.
	FixUpDecreasedNodes(old_quaterne_used_end);
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

void form::NodeBuffer::DeinitChildren(Node * children)
{
	ASSERT(children->GetParent()->GetChildren() == children);
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

void form::NodeBuffer::SubstituteChildren(Node * substitute, Node * original)
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

// Repair the pointers that point TO this node.
void form::NodeBuffer::RepairChild(Node & child)
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

void form::NodeBuffer::IncreaseNodes(Quaterna * new_quaterne_used_end)
{
	// Verify that the input is indeed a decrease.
	VerifyArrayElement(new_quaterne_used_end, quaterne_used_end, quaterne_end + 1);
	
	// Increasing the target number of nodes is simply a matter of setting a value. 
	// The target pointer now point_buffer into the range of unused quaterne at the end of the array.
	quaterne_used_end_target = new_quaterne_used_end;
	
	VerifyObject(* this);
}

// Reduce the number of used nodes and, accordingly, the number of used quaterne.
// This is quite an involved and painful process which sometimes fails half-way through.
void form::NodeBuffer::DecreaseNodes(Quaterna * new_quaterne_used_end)
{
	Quaterna * old_quaterne_used_end = quaterne_used_end;

	// First decrease the number of quaterne. This is the bit that sometimes fails.
	DecreaseQuaterna(new_quaterne_used_end);
	
	// Was there any decrease at all?
	if (old_quaterne_used_end == quaterne_used_end)
	{
		//ASSERT(false);	// This isn't deadly fatal but serious enough that I'd like to know it happens.
		return;
	}
	
	// Because the nodes aren't in the correct order, decreasing them is somewhat more tricky.
	FixUpDecreasedNodes(old_quaterne_used_end);
	
	VerifyObject (* this);
}

void form::NodeBuffer::DecreaseQuaterna(Quaterna * new_quaterne_used_end)
{
	// Verify that the input is indeed a decrease.
	VerifyArrayElement(new_quaterne_used_end, quaterne, quaterne_used_end + 1);
	
	// Loop through used quats backwards from far end.
	do 
	{
		Quaterna & q = quaterne_used_end [- 1];
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
		
		-- quaterne_used_end;
	}	
	while (quaterne_used_end > new_quaterne_used_end);
	
	// Both quaterne_sorted_end and quaterne_used_end_target 
	// must be equal to quaterne_used_end at this point. 
	if (quaterne_sorted_end > quaterne_used_end)
	{
		quaterne_sorted_end = quaterne_used_end;
	}
	
	// Target is really for increasing the target during churn.
	quaterne_used_end_target = quaterne_used_end;
}

#define WIP 0
void form::NodeBuffer::FixUpDecreasedNodes(Quaterna * old_quaterne_used_end)
{
	// From the new used quaterna value, figure out new used node value.
#if (WIP == 0)
	auto new_num_quaterne_used = quaterne_used_end - quaterne;
	auto new_num_nodes_used = new_num_quaterne_used << 2;	// 4 nodes per quaterna
	nodes_used_end = nodes + new_num_nodes_used;
#endif
	VerifyArrayElement(nodes_used_end, nodes, nodes_end + 1);
	
	// Use this pointer to walk down the used quaterne array.
	Quaterna * used_quaterna = quaterne_used_end;
	
	// For all the quaterne that have been freed up by the reduction...
	for (Quaterna * unused_quaterna = old_quaterne_used_end - 1; unused_quaterna >= quaterne_used_end; -- unused_quaterna)
	{
		ASSERT(! unused_quaterna->IsInUse());
		
		Node * unused_nodes = unused_quaterna->nodes;
		
		// Is a quatern past the end of used quaterne
		// pointing to a node before the end of used nodes?
		if (unused_nodes < nodes_used_end)
		{
			// It needs to point to a quad of nodes past the end of used nodes.
			// There must be a corresponding quad of nodes in such a position that is in use.
			// They must be swapped.
			
			// Find the used quaterne whose nodes are past the end of the used range.
			Node * substitute_nodes;
			do
			{
				-- used_quaterna;
				ASSERT(used_quaterna >= quaterne);
				substitute_nodes = used_quaterna->nodes;
			}
			while (substitute_nodes < nodes_used_end);

			SubstituteChildren(unused_nodes, substitute_nodes);
			std::swap(used_quaterna->nodes, unused_quaterna->nodes);
		}
		
#if (WIP == 1)
		// Finally steam-roll over whatever the nodes pointer was because 
		// we want unused quaterne to point to their equivalent in the node array;
		nodes_used_end -= 4;
		unused_quaterna->nodes = nodes_used_end;
#endif
	}

	// Finally, the nodes were swapped between used and unused quats in any old order.
	// But the unused nodes need to line up with the unused quaterne.
	std::sort(quaterne_used_end, old_quaterne_used_end, QuaternaSortUnused);
	
	VerifyObject(* this);
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachNode(FUNCTOR f, size_t step_size, bool parallel)
{
	if (nodes_used_end == nodes)
	{
		return;
	}
	
	if (! parallel)
	{
		core::for_each <form::Node *, FUNCTOR> (nodes, nodes_used_end, f);
	}
	else 
	{
		smp::for_each <form::Node *, FUNCTOR>(nodes, nodes_used_end, f, step_size, -1);
	}
}

template <typename FUNCTOR> 
void form::NodeBuffer::ForEachQuaterna(FUNCTOR f, size_t step_size, bool parallel)
{
	if (quaterne_used_end == quaterne)
	{
		return;
	}

	if (! parallel && step_size == 1)
	{
		core::for_each<form::Quaterna *, FUNCTOR>(quaterne, quaterne_used_end, f);
	}
	else
	{
		smp::for_each<form::Quaterna *, FUNCTOR>(quaterne, quaterne_used_end, f, step_size, -1);
	}
}
