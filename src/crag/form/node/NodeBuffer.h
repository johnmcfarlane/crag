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
#include "core/Mutex.h"

#include "geom/Vector3.h"

#include "form/score/CalculateNodeScoreCpuKernel.h"
#include "form/score/CalculateNodeScoreGpuKernel.h"


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
	// TODO: This class needs to be broken up further. 
	class NodeBuffer
	{
		//friend class ExpandNodeFunctor;
	public:
		
		// Constants
		enum {
			max_num_quaterna = 262144,
			max_num_nodes = max_num_quaterna << 2,
			max_num_verts = max_num_nodes * 2,
			max_num_indices = max_num_verts * 3
		};
		
		// Member functions
		NodeBuffer(int target_num_nodes);
		~NodeBuffer();
		
#if VERIFY
		//void VerifyRanking(Ranking const * ranking) const;
		void Verify() const;
		void VerifyUsed(Quaterna const & q) const;
		void VerifyUnused(Quaterna const & q) const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(NodeBuffer);
		
		PointBuffer & GetPoints() { return points; }
		
		int GetNumQuaternaUsed() const;
		int GetNumQuaternaUsedTarget() const;
		
		// Must be a multiple of four.
		void SetNumQuaternaUsedTarget(int n);	
		
		void LockTree() const;
		void UnlockTree() const;
		
		void Tick(Ray3 const & camera_ray_relative);
		void OnReset();
		void ResetNodeOrigins(Vector3 const & origin_delta);
	private:
		void InitKernel();
		void InitQuaterna(Quaterna const * end);
		void UpdateNodeScores(Ray3 const & camera_ray_relative);
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
		bool ExpandNode(Node & node);
		bool ExpandNode(Node & node, Quaterna & children_quaterna);
		void CollapseNode(Node & node);
	private:
		static bool InitChildGeometry(Node const & parent, Node * children_copy, Shader & shader);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		static void SubstituteChildren(Node * substitute, Node * children);
		static void RepairChild(Node & child);

		void IncreaseNodes(Quaterna * new_quaterna_used_end);
		void DecreaseNodes(Quaterna * new_quaterna_used_end);
		
		void DecreaseQuaterna(Quaterna * new_quaterna_used_end);
		void FixUpDecreasedNodes(Quaterna * old_quaterna_used_end);
		
		// TODO: Parallelize
		template <class FUNCTOR> void ForEachNode(FUNCTOR & f, int step_size = 1024);
		
		// Types

		// The fixed-size array of node groups, used and unused.
		Node * const nodes;	// [max_num_nodes]
		
		Node * nodes_used_end;			// end of buffer of actually used nodes
		Node const * const nodes_end;
		
		// An array of used nodes in ascending order of score.
		// TODO: Plural of quaterna is quaterne. Maybe quaterna is just quadruplet in forin. 
		// TODO: Maybe store nodes in sets of four and make them the Qua...whatevers and the quaterna something with sorted in the title.
		Quaterna * const quaterna;		// [max_num_quaterna]

		Quaterna * quaterna_used_end;			// end of buffer of actually used quaterna
		Quaterna * quaterna_sorted_end;			// end of buffer of actually used nodes
		Quaterna * quaterna_used_end_target;	// where we'd like quaterna_used_end to be; cannot be less than used_end
		Quaterna const * const quaterna_end;
		
		// Pool of vertices from which to take the corners of nodes.
		// TODO: NodeBuffer and PointBuffer could maybe be owned by the same, parent class. 
		PointBuffer points;
		
		// When locked, the structure of the node trees cannot be changed;
		// No new children can be added and no old ones removed.
		mutable core::Mutex tree_mutex;
		
#if (USE_OPENCL)
		CalculateNodeScoreCpuKernel * cpu_kernel;
		CalculateNodeScoreGpuKernel * gpu_kernel;
#endif
	};
	
}
