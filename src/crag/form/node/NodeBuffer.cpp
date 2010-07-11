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
#include "Polyhedron.h"
#include "Node.h"
#include "Quaterna.h"
#include "Shader.h"

#include "form/score/CalculateNodeScoreFunctor.h"

#include "form/scene/Mesh.h"

#include "cl/Singleton.h"

#include "core/ConfigEntry.h"
#include "core/floatOps.h"

#include <algorithm>


namespace 
{
//CONFIG_DEFINE (min_sorted_nodes, int, 100);	// At least the first <min_sorted_nodes> in nodes will be sorted (and the lowest of all the array) for sure.
//CONFIG_DEFINE (prop_sorted_nodes, float, .01f);
//CONFIG_DEFINE (recycle_to_sorted_coefficient, float, 1.5f);

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


#if (USE_OPENCL)
#define CPU_KERNEL_CL_FILENAME "src/crag/form/score/CalculateNodeScoreCpu.cl"
#define GPU_KERNEL_CL_FILENAME "src/crag/form/score/CalculateNodeScoreGpu.cl"
#define CPU_KERNEL_CPP_FILENAME "src/crag/form/score/CalculateNodeScoreCpuString.h"
#define GPU_KERNEL_CPP_FILENAME "src/crag/form/score/CalculateNodeScoreGpuString.h"
#endif


form::NodeBuffer::NodeBuffer(int target_num_quaterna)
//: nodes(new Node [max_num_nodes])
: nodes(reinterpret_cast<Node *>(Allocate(sizeof(Node) * max_num_nodes, 128)))
, nodes_used_end(nodes)
, nodes_end(nodes + max_num_nodes)
, quaterna(new Quaterna [max_num_quaterna])
, quaterna_used_end(quaterna)
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
	Assert(GetNumQuaternaUsed() == 0);
	
	delete quaterna;

	//delete nodes;
	Free(nodes);
}

#if VERIFY
void form::NodeBuffer::Verify() const
{
	Node const * nodes_used_end_target = nodes + (quaterna_used_end_target - quaterna) * 4;
	VerifyArrayElement(nodes_used_end, nodes, nodes_end + 1);	
	VerifyArrayElement(nodes_used_end_target, nodes_used_end, nodes_end + 1);	

	VerifyArrayElement(quaterna_used_end, quaterna, quaterna_end + 1);	
	VerifyArrayElement(quaterna_used_end_target, quaterna_used_end, quaterna_end + 1);	

	int num_nodes_used = nodes_used_end - nodes;
	int num_quaterna_used = quaterna_used_end - quaterna;

	VerifyTrue(num_nodes_used == num_quaterna_used * 4);
	
	//VerifyObject(points);
	
	for (Quaterna const * q = quaterna; q != quaterna_end; ++ q) 
	{
		Node const * n = q->nodes;

		Node const * parent = q->nodes[0].parent;

		if (q < quaterna_used_end)
		{
			VerifyArrayElement(n, nodes, nodes_used_end);
			VerifyTrue(parent->score == q->parent_score);
			
			for (int i = 0; i < 4; ++ i)
			{
				form::Node const & sibling = q->nodes[i];
				
				// All four siblings should have the same parent.
				VerifyTrue(q->nodes[i].parent == parent);
				if (sibling.children != nullptr)
				{
					VerifyArrayElement(sibling.children, nodes, nodes_used_end);
				}
			}
		}
		else 
		{
			VerifyTrue(n - nodes == (q - quaterna) * 4);
			
			for (int i = 0; i < 4; ++ i)
			{
				form::Node const & sibling = q->nodes[i];
				
				VerifyTrue(sibling.parent == nullptr);
				VerifyTrue(sibling.children == nullptr);
			}
		}
		
		VerifyObject(* q);
	}
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

	Quaterna * new_target = Clamp<Quaterna *>(quaterna + n, quaterna + 0, const_cast<Quaterna *>(quaterna_end));
	
	if (new_target == quaterna_used_end) 
	{
		quaterna_used_end_target = new_target;
	}
	else if (new_target > quaterna_used_end) 
	{
		IncreaseNodes(new_target);
	}
	else if (new_target < quaterna_used_end)
	{
		LockTree();
		
		if (! DecreaseNodes(new_target)) 
		{
			//Assert(false);
			
			//std::sort(quaterna, quaterna_available_end, SortByScoreExtreme);
			//DecreaseAvailableRankings(new_rankings_available_end);
		}
		
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

void form::NodeBuffer::Tick(Vector3 const & relative_camera_pos, Vector3 const & camera_dir)
{
	UpdateNodeScores(relative_camera_pos, camera_dir);
	UpdateParentScores();
	SortNodes();
	
	for (int timeout = 1; timeout; -- timeout) {
		ChurnNodes();
	}
}

void form::NodeBuffer::InitKernel()
{
#if (USE_OPENCL)
	cl::Singleton const & cl_singleton = cl::Singleton::Get();
	
#if ! defined(NDEBUG)
	char const * cl_filename;
	char const * cpp_filename;
	switch (cl_singleton.GetDeviceType())
	{
		default:
			assert(false);	// unrecognized device type
		case CL_DEVICE_TYPE_ALL:
			return;
			
		case CL_DEVICE_TYPE_CPU:
			cl_filename = CPU_KERNEL_CL_FILENAME;
			cpp_filename = CPU_KERNEL_CPP_FILENAME;
			break;
			
		case CL_DEVICE_TYPE_GPU:
			cl_filename = GPU_KERNEL_CL_FILENAME;
			cpp_filename = GPU_KERNEL_CPP_FILENAME;
			break;
	}
	
	char * source_string = source_string = cl::Kernel::LoadClFile(cl_filename);
	cl::Kernel::SaveCFile(cpp_filename, source_string, "kernel_source");
	delete [] source_string;
#endif
	
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
	quaterna_used_end = quaterna;
	
	// Half the target number of nodes.
	// Probably not a smart idea.
	//quaterna_used_end_target -= (quaterna_used_end_target - quaterna) >> 1;
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

void form::NodeBuffer::UpdateNodeScores(Vector3 const & relative_camera_pos, Vector3 const & camera_dir)
{
#if (USE_OPENCL)
	if (cpu_kernel != nullptr)
	{
		cpu_kernel->Process(nodes, nodes_used_end, relative_camera_pos);
	}
	else if (gpu_kernel != nullptr)
	{
		gpu_kernel->Process(nodes, nodes_used_end, relative_camera_pos);
	}
	else 
#endif
	{
		CalculateNodeScoreFunctor f(relative_camera_pos, camera_dir);
		ForEachNode(f);
	}
}

void form::NodeBuffer::UpdateParentScores()
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
}

// Algorithm to sort nodes array. 
// TODO: Exploit is_sorted somehow! 
void form::NodeBuffer::SortNodes()
{
#if 1
	// Sure and steady ... and slow!
	std::sort(quaterna, quaterna_used_end, SortByScore);
	
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
	
	// Get the score to pass to GetWorstQuaterna.
	// We don't want to nuke a quaterna with a worse one, hence using the node score.
	// But also, there's a slim chance that node's parent has a worse score than node's
	// and we can't end up finding a quaterna that includes node as node's new children
	// because that would cause a time paradox in the fabric of space.
	float score = node.score;
	Node * parent = node.parent;
	if (parent != nullptr) 
	{
		float parent_score = parent->score;
		if (parent_score < score) 
		{
			score = parent_score;
		}
	}
	
	Quaterna * worst_quaterna = GetWorstQuaterna(score);
	if (worst_quaterna == nullptr) {
		return false;
	}
	
	Node * worst_children = worst_quaterna->nodes;
	Assert(worst_children + 0 != & node);
	Assert(worst_children + 1 != & node);
	Assert(worst_children + 2 != & node);
	Assert(worst_children + 3 != & node);
		
	Shader & shader = GetModel(node).GetShader();
	InitMidPoints(node, shader);
	
	// Work with copy of children until it's certain that expansion is going to work.
	Node children_copy[4];	
	if (! InitChildGeometry(node, children_copy, shader)) {
		// Probably, a child is too small to be represented using float accuracy.
		return false;
	}
	
	LockTree();

	// Deinit children.
	if (worst_quaterna->IsInUse()) {
		DeinitChildren(worst_children);
		Assert(worst_quaterna->nodes[0].score == 0);
	}
	Assert(worst_quaterna->nodes[0].score == 0);
	
	worst_children[0] = children_copy[0];
	worst_children[1] = children_copy[1];
	worst_children[2] = children_copy[2];
	worst_children[3] = children_copy[3];

	node.children = worst_children;
	InitChildPointers(node);
	
	UnlockTree();
	
	worst_quaterna->parent_score = score;
	BubbleSortUp(worst_quaterna);
	
	VerifyObject(* worst_quaterna);
	//VerifyObject(*this);
	
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

// Makes sure node's three mid-points are non-null or returns false.
void form::NodeBuffer::InitMidPoints(Node & node, Shader & shader)
{
	Node::Triplet * triple = node.triple;
	
	// Make sure all mid-points exist.
	for (int triplet_index = 0; triplet_index < 3; ++ triplet_index)
	{
		// If the mid-point does not already exist,
		Node::Triplet & t = triple[triplet_index];
		if (t.mid_point == nullptr)
		{
			// create the mid-point, share it with the cousin
			Node & cousin = ref(t.cousin);
			t.mid_point = cousin.triple[triplet_index].mid_point = points.Alloc();
			Assert (t.mid_point != nullptr);	// decrease MAX_NUM_NODES (relative to MAX_NUM_VERTS)
			
			// and initialize it's position etc..
			shader.InitMidPoint(triplet_index, node, cousin, * t.mid_point);
		}
	}
}

bool form::NodeBuffer::InitChildGeometry(Node & parent, Node * children, Shader & shader)
{
	// TODO: Hard-code this and remove GetChildCorners.
	for (int child_index = 0; child_index < 4; ++ child_index) {
		Point * child_corners[3];
		parent.GetChildCorners(child_index, child_corners);
		
		Node & child = children [child_index];
		
		child.triple[0].corner = child_corners[0];
		child.triple[1].corner = child_corners[1];
		child.triple[2].corner = child_corners[2];
		if (child.InitGeometry() == false) {
			return false;
		}
		
		child.score = 0;	// std::numeric_limits<float>::max();
	}
	
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
	
	VerifyObject(node);
}

void form::NodeBuffer::SubstituteChildren(Node * substitute, Node * original)
{
	Node * parent = original->parent;
	
	if (parent != nullptr) {
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

void form::NodeBuffer::IncreaseNodes(Quaterna * new_target)
{
	Assert (new_target > quaterna_used_end);
	Assert (new_target <= quaterna_end);
	
	quaterna_used_end_target = new_target;
}

// TODO: This can be improved a lot.
// TODO: Should sweep all surviving quats first and do all the substitutions in the correct order.
// TODO: That way, the unavailable quats can keep their node pointers.
bool form::NodeBuffer::DecreaseNodes(Quaterna * new_target)
{
	Assert (new_target >= quaterna);
	Assert (new_target < quaterna_used_end);
	
	Quaterna * old_quaterna_used_end = quaterna_used_end;
	
	int new_num_quaterna = new_target - quaterna;
	int new_num_nodes = new_num_quaterna << 2;
	Node * new_nodes_available_end = nodes + new_num_nodes;
	Quaterna * replaced_quaterna = quaterna;
	
	bool success = true;

	do 
	{
		Quaterna & q = quaterna_used_end [- 1];
		Node * children = q.nodes;
		
		if (q.IsInUse()) 
		{
			if (! q.IsLeaf()) 
			{
				success = false;
				//std::cerr << "success = false\n";
				//Assert(false);
				break;
			}
			
			q.parent_score = -1;
			
			DeinitChildren(children);
		}

		Assert(children[0].children == 0);
		Assert(children[1].children == 0);
		Assert(children[2].children == 0);
		Assert(children[3].children == 0);
		
		// Are the children of this out-of-range quaterna in range?
		if (children < new_nodes_available_end) 
		{
			Node * replaced_children;
			
			// If so, find an in-range quaterna with out-of-range nodes.
			while ((replaced_children = replaced_quaterna->nodes) < new_nodes_available_end) 
			{
				++ replaced_quaterna;
				Assert(replaced_quaterna < new_target);
			}
			
			SubstituteChildren(children, replaced_children);
			
			replaced_quaterna->nodes = children;
			Assert(replaced_quaterna->nodes < new_nodes_available_end);
			
			q.nodes = replaced_children;
		}
		Assert(q.nodes >= new_nodes_available_end);
		
		nodes_used_end -= 4;
		-- quaterna_used_end;
	}	while (quaterna_used_end > new_target);
	
	quaterna_used_end_target = quaterna_used_end;
	
	std::sort(quaterna_used_end, old_quaterna_used_end, SortByNodes);
	
	return success;
}

form::Quaterna * form::NodeBuffer::GetWorstQuaterna(float parent_score)
{
	Assert(parent_score >= 0);
	
	if (quaterna_used_end != quaterna_used_end_target)
	{
		nodes_used_end += 4;
		return quaterna_used_end ++;
	}
	
	if (quaterna_used_end > quaterna) 
	{
		Quaterna & replacement = quaterna_used_end [- 1];
		if (replacement.IsSuitableReplacement(parent_score)) 
		{
			return & replacement;
		}
	}
	
	return nullptr;
}

// Reposition given, available quartet assuming it's score is higher then its position suggests.
// Thus is can only go up, i.e. towards the start of the array.
void form::NodeBuffer::BubbleSortUp(Quaterna * quartet)
{
	Quaterna q = * quartet;

	Quaterna * iterator = quartet;
	while (true) 
	{
		-- iterator;
		
		if (iterator >= quaterna)
		{
			if (iterator->parent_score <= q.parent_score)
			{
				continue;
			}
		}
		
		break;
	}
	
	++ iterator;
	
	if (iterator < quartet) 
	{
		memmove(iterator + 1, iterator, reinterpret_cast<char *> (quartet) - reinterpret_cast<char *> (iterator));
		(* iterator) = q;
	}
}
	
template <class FUNCTOR> void form::NodeBuffer::ForEachNode(FUNCTOR & f, int step_size)
{
	ForEachNode(f, nodes, nodes_used_end, step_size);
}

template <class FUNCTOR> void form::NodeBuffer::ForEachNode(FUNCTOR & f, Node * begin, Node * end, int step_size)
{
	int total_num_nodes = end - begin;
	int full_steps = total_num_nodes / step_size;
	
	// main pass
	for (int step = 0; step < full_steps; step ++)
	{
		Node * sub_begin = begin + step * step_size;
		Node * sub_end = sub_begin + step_size;
		ForEachNode_Sub(f, sub_begin, sub_end);
	}

	// remainder
	Node * sub_begin = begin + full_steps * step_size;
	Node * sub_end = end;
	Assert(sub_end - sub_begin == total_num_nodes % step_size);

	if (sub_begin < sub_end)
	{
		ForEachNode_Sub(f, sub_begin, sub_end);
	}
}

template <class FUNCTOR> void form::NodeBuffer::ForEachNode_Sub(FUNCTOR & f, Node * begin, Node * end)
{
	// Pre-fetch the actual nodes.
	// TODO: Find out if this does any good at all. 
	PrefetchArray(begin, end);

	// Do any additional pre-fetching desired by the functor.
	if (f.PerformPrefetchPass()) {
		for (Node * iterator = begin; iterator != end; ++ iterator) {
			f.OnPrefetchPass(* iterator);
		}
	}
	
	// Now do the work.
	for (Node * iterator = begin; iterator != end; ++ iterator) {
		f(* iterator);
	}
}

