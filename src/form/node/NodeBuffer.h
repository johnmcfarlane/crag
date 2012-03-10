//
//  NodeBuffer.h
//  crag
//
//  Created by John on 2/17/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "PointBuffer.h"

#include "core/debug.h"

#include "smp/ReadersWriterMutex.h"
#include "smp/Vector.h"

#include "geom/Vector3.h"

#include "form/node/CalculateNodeScoreFunctor.h"


namespace form
{
	
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	
	class Mesh;
	class Node;
	class Shader;
	
	struct Quaterna;
	
	template <size_t MAX_NUM_NODES> class ExpandNodeParallelFunctor;

	
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
			// tweakables
			min_num_portions = 64,
			max_num_portions = 65536,

			// derivations
			num_nodes_per_quaterna = 4,
			portion_num_nodes = num_nodes_per_quaterna,
			
			min_num_quaterne = min_num_portions,
			max_num_quaterne = max_num_portions,

			max_num_nodes = max_num_quaterne * num_nodes_per_quaterna,
			max_num_verts = max_num_nodes * 2,
			max_num_indices = max_num_verts * 3
		};

		friend class ExpandNodeParallelFunctor <max_num_nodes>;

		// Member functions
		NodeBuffer();
		~NodeBuffer();
		
#if defined(VERIFY)
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
		
		// returns 0 if there are none
		float GetMinParentScore() const;
		
		// Must be a multiple of four.
		void SetNumQuaternaUsedTarget(int n);	
		
		void ReadLockTree() const;
		void ReadUnlockTree() const;
		void WriteLockTree() const;
		void WriteUnlockTree() const;
		
		void Tick(Ray3 const & new_camera_ray);
		void OnReset();
		void ResetNodeOrigins(Vector3 const & origin_delta);
	private:
		void InitQuaterna(Quaterna const * end);
		
		void UpdateNodes();
		void UpdateNodeScores();
		void UpdateQuaterna();
		void UpdateQuaternaScores();
		void SortQuaterna();
		bool ChurnNodes();
	public:
		
		void GenerateMesh(Mesh & mesh);
		
		bool IsChildNode(Node const & node) const;
		bool IsValidNodePointer(Node const * node) const;
		int GetNodeIndex(Node const & node) const;
		
		///////////////////////////////////////////////////////
		// Node-related members.
		static bool IsNodeChurnIntensive() { return true; }
		float GetWorseReplacableQuaternaScore() const;
		bool ExpandNode(Node & node);
		bool ExpandNode(Node & node, Quaterna & children_quaterna);
		void CollapseNodes(Node & root);
	private:
		void CollapseNode(Node & node);
		static bool InitChildGeometry(Node const & parent, Node * children_copy);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		static void SubstituteChildren(Node * substitute, Node * children);
		static void RepairChild(Node & child);

		void IncreaseNodes(Quaterna * new_quaterne_used_end);
		void DecreaseNodes(Quaterna * new_quaterne_used_end);
		
		void DecreaseQuaterna(Quaterna * new_quaterne_used_end);
		void FixUpDecreasedNodes(Quaterna * old_quaterne_used_end);

		template <typename FUNCTOR> 
		void ForEachNode(FUNCTOR f, size_t step_size, bool parallel = false);
		
		template <typename FUNCTOR> 
		void ForEachQuaterna(FUNCTOR f, size_t step_size = 1, bool parallel = false);

		// Types

		// The fixed-size array of node groups, used and unused.
		Node * nodes;	// [max_num_nodes]
		
		Node * nodes_used_end;			// end of buffer of actually used nodes
		Node const * const nodes_end;
		
		// An array of used nodes in ascending order of score.
		Quaterna * const quaterne;		// [max_num_quaterne]

		Quaterna * quaterne_sorted_end;			// end of the range the we know is sorted
		Quaterna * quaterne_used_end;			// end of buffer of actually used quaterna
		Quaterna * quaterne_used_end_target;	// where we'd like quaterne_used_end to be; cannot be less than used_end
		Quaterna const * const quaterne_end;
		
		// Pool of vertices from which to take the corners of nodes.
		PointBuffer point_buffer;
		
		// When locked, the structure of the node trees cannot be changed;
		// No new children can be added and no old ones removed.
		mutable smp::ReadersWriterMutex tree_mutex;
		
		CalculateNodeScoreFunctor node_score_functor;
		Ray3 cached_node_score_ray;	// ray used when last the node buffer's scores were recalculated en masse. 
	};
	
}
