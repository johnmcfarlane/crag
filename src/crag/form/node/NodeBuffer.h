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
	
	class NodeBuffer
	{
		friend class ExpandNodeFunctor;
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
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(NodeBuffer);
		
		PointBuffer & GetPoints() { return points; }
		
		int GetNumQuaternaUsed() const;
		int GetNumQuaternaUsedTarget() const;
		
		// Must be a multiple of four.
		void SetNumQuaternaUsedTarget(int n);	
		
		void LockTree() const;
		void UnlockTree() const;
		
		void Tick(Vector3 const & relative_camera_pos, Vector3 const & camera_dir);
		void OnReset();
	private:
		void InitKernel();
		void InitQuaterna(Quaterna const * end);
		void UpdateNodeScores(Vector3 const & relative_camera_pos, Vector3 const & camera_dir);
		void UpdateParentScores();
		void SortNodes();
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
		void CollapseNode(Node & node);
	private:
		void InitMidPoints(Node & node, Shader & shader);
		static bool InitChildGeometry(Node & parent, Node * children_copy, Shader & shader);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		static void SubstituteChildren(Node * substitute, Node * children);
		static void RepairChild(Node & child);

		void IncreaseNodes(Quaterna * new_target);
		bool DecreaseNodes(Quaterna * new_target);
		
		Quaterna * GetWorstQuaterna(float parent_score);
		void BubbleSortUp(Quaterna * quaterna);

		// TODO: Parallelize
		template <class FUNCTOR> void ForEachNode(FUNCTOR & f, int step_size = 1024);
		
		// Types

		// The fixed-size array of node groups, used and unused.
		Node * const nodes;	// [max_num_nodes]
		
		Node * nodes_used_end;			// end of buffer of actually used nodes
		Node const * const nodes_end;
		
		// An array of used nodes in ascending order of score.
		// TODO: Plural of quaterna is quaterne. Maybe quaterna is just quadruplet in forin. 
		Quaterna * const quaterna;		// [max_num_quaterna]

		Quaterna * quaterna_used_end;			// end of buffer of actually used quaterna
		Quaterna * quaterna_used_end_target;	// where we'd like quaterna_used_end to be; cannot be less than used_end
		Quaterna const * const quaterna_end;
		
		// Pool of vertices from which to take the corners of nodes.
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
