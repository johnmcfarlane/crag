/*
 *  NodeBuffer.h
 *  Crag
 *
 *  Created by John on 2/17/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "PointBuffer.h"

#include "core/debug.h"
#include "smp/Mutex.h"

#include "geom/Vector3.h"

//#include "form/score/CalculateNodeScoreCpuKernel.h"
//#include "form/score/CalculateNodeScoreGpuKernel.h"
#include "form/score/CalculateNodeScoreFunctor.h"


namespace form
{
	class Mesh;
	class Node;
	class Shader;
	
	// Declared in .cpp
	struct Quaterna;
	
	// There's no easy way to explain all the things this class does.
	// Some of the most complicated code lives in this class.
	// In short, this is a full set of nodes needed to describe all the formations in a scene. 
	// Nodes are grouped into fours because they are allocated in this number.
	// They are also sorted in descending order of the score of their parents using Quaterna. 
	// This way low-priority leaves can be removed and high-priority parents can grow. 
	// TODO: This class needs to be broken up further. NodeSet contains PointBuffer, NodeBuffer and QuaternaBuffer perhaps?
	class NodeBuffer
	{
		OBJECT_NO_COPY (NodeBuffer);
		
	public:
		
		// Constants
		enum {
			min_num_quaterna = 256,
			max_num_quaterna = 262144,
			max_num_nodes = max_num_quaterna << 2,
			max_num_verts = max_num_nodes * 2,
			max_num_indices = max_num_verts * 3
		};
		
		// Member functions
		NodeBuffer();
		~NodeBuffer();
		
#if VERIFY
		//void VerifyRanking(Ranking const * ranking) const;
		void Verify() const;
		void VerifyUsed(Quaterna const & q) const;
		void VerifyUnused(Quaterna const & q) const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(NodeBuffer);
		
		PointBuffer & GetPoints() { return point_buffer; }
		
		int GetNumNodesUsed() const;
		int GetNumQuaternaUsed() const;
		int GetNumQuaternaUsedTarget() const;
		
		// Must be a multiple of four.
		void SetNumQuaternaUsedTarget(int n);	
		
		void LockTree() const;
		void UnlockTree() const;
		
		void Tick(Ray3 const & new_camera_ray);
		void OnReset();
		void ResetNodeOrigins(Vector3 const & origin_delta);
	private:
		void InitKernel();
		void InitQuaterna(Quaterna const * end);
		
		void UpdateNodes();
		void UpdateNodeScores();
		void UpdateQuaternaScores();
		void SortQuaterna();
		bool ChurnNodes();
	public:
		
		void GenerateMesh(Mesh & mesh);
		
		bool IsChildNode(Node const & node) const;
		bool IsValidNodePointer(Node const * node) const;
		int GetNodeIndex(Node const & node) const;
		//bool IsAvailable(class Ranking const * ranking) const;
		
		///////////////////////////////////////////////////////
		// Node-related members.
		static bool IsNodeChurnIntensive() { return true; }
		bool ExpandNode(Node & node);
		bool ExpandNode(Node & node, Quaterna & children_quaterna);
		void CollapseNode(Node & node);
	private:
		static bool InitChildGeometry(Node const & parent, Node * children_copy);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		static void SubstituteChildren(Node * substitute, Node * children);
		static void RepairChild(Node & child);

		void IncreaseNodes(Quaterna * new_quaterna_used_end);
		void DecreaseNodes(Quaterna * new_quaterna_used_end);
		
		void DecreaseQuaterna(Quaterna * new_quaterna_used_end);
		void FixUpDecreasedNodes(Quaterna * old_quaterna_used_end);

		template <typename FUNCTOR> 
		void ForEachNode(size_t step_size, FUNCTOR f, bool parallel);
		
		template <typename FUNCTOR1, typename FUNCTOR2> 
		void ForEachNode(size_t step_size, FUNCTOR1 f1, FUNCTOR2 f2, bool parallel);

		template <typename FUNCTOR> 
		void ForEachQuaterna(size_t step_size, FUNCTOR f, bool parallel);

		// Types

		// The fixed-size array of node groups, used and unused.
		Node * const nodes;	// [max_num_nodes]
		
		Node * nodes_used_end;			// end of buffer of actually used nodes
		Node const * const nodes_end;
		
		// An array of used nodes in ascending order of score.
		// TODO: Plural of quaterna is quaterne. Maybe quaterna is just quadruplet in forin languich. 
		// TODO: quadrisection: The action of dividing something into four parts.
		// TODO: Maybe store nodes in sets of four and make them the Qua...whatevers and the quaterna something with sorted in the title.
		Quaterna * const quaterna;		// [max_num_quaterna]

		Quaterna * quaterna_sorted_end;			// end of the range the we know is sorted
		Quaterna * quaterna_used_end;			// end of buffer of actually used quaterna
		Quaterna * quaterna_used_end_target;	// where we'd like quaterna_used_end to be; cannot be less than used_end
		Quaterna const * const quaterna_end;
		
		// Pool of vertices from which to take the corners of nodes.
		// TODO: NodeBuffer and PointBuffer could maybe be owned by the same, parent class. 
		PointBuffer point_buffer;
		
		// When locked, the structure of the node trees cannot be changed;
		// No new children can be added and no old ones removed.
		mutable smp::Mutex tree_mutex;
		
		CalculateNodeScoreFunctor node_score_functor;
		Ray3 cached_node_score_ray;	// ray used when last the node buffer's scores were recalculated en masse. 
		
#if defined(USE_OPENCL)
		CalculateNodeScoreCpuKernel * cpu_kernel;
		CalculateNodeScoreGpuKernel * gpu_kernel;
#endif
	};
	
}
