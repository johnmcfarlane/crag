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

//#include "Node.h"
#include "VertexBuffer.h"

#include "core/debug.h"
#include "core/Vector3.h"


namespace form
{
	class Mesh;
	class Node;
	class RootNode;
	class Shader;
	
	// Declared in .cpp
	struct Quaterna;
	
	class NodeBuffer
	{
		friend class ExpandNodeFunctor;
	public:
		
		// Constants
		enum {
			max_num_verts = 500000,
			max_num_indices = max_num_verts * 3,
			max_num_nodes = (max_num_verts * 45) / 100,
			max_num_quaterna = max_num_nodes >> 2
		};
		
		// Member functions
		NodeBuffer();
		~NodeBuffer();
		
#if VERIFY
		//void VerifyRanking(Ranking const * ranking) const;
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(NodeBuffer);
		
		VertexBuffer & GetVertices() { return vertices; }
		
		int GetNumQuaternaAvailable() const;
	//	int GetNumNodesInUse() const;
//		float GetChurnMetric() const;
//		void ResetChurnCounter(); 
		
		// Must be a multiple of four.
		void SetNumQuaternaAvailable(int n);	
		
		void Tick(Vector3f const & relative_camera_pos);
	private:
		void UpdateNodeScores(Vector3f const & relative_camera_pos);
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
		bool ExpandNode(Node & node/*, Shader & shader*/);
		void CollapseNode(Node & node);
	private:
		void InitMidPoints(Node & node, Shader & shader);
		static class Shader & GetShader(Node & node);
		static class RootNode & GetRootNode(Node & node);
		static bool InitChildGeometry(Node & parent, Node * children_copy, Shader & shader);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		static void SubstituteChildren(Node * substitute, Node * children);
		static void RepairChild(Node & child);

		void IncreaseAvailableRankings(Quaterna * new_quaterna_available_end);
		bool DecreaseAvailableRankings(Quaterna * new_quaterna_available_end);
		
		Quaterna * GetWorstQuaterna(float parent_score);
		void BubbleSortUp(Quaterna * quaterna);

		// TODO: Parallelize
		template <class FUNCTOR> void ForEachNode(FUNCTOR & f, int step_size = 128);
		template <class FUNCTOR> void ForEachNode(FUNCTOR & f, Node * begin, Node * end, int step_size = 128);

		template <class FUNCTOR> void ForEachNode_Sub(FUNCTOR & f, Node * begin, Node * end);
		
		// Types

		// The fixed-size array of node groups, used and unused.
		Node * nodes;	// [max_num_nodes]
		
		Node * nodes_available_end;
		
		// An array of used nodes in ascending order of score.
		Quaterna * quaterna;		// [max_num_quaterna]
		
		Quaterna * quaterna_available_end;
		Quaterna * quaterna_end;
		
		// Pool of vertices from which to take the corners of nodes.
		VertexBuffer vertices;
		
		//int num_nodes_in_use;
		//int num_rankings_recycled;
		//int num_rankings_recycled_this_tick;
	};
	
}
