//
//  Surrounding.h
//  crag
//
//  Created by John on 3013-11-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "PointBuffer.h"
#include "NodeBuffer.h"

#include "core/debug.h"

#include "smp/vector.h"

#include "CalculateNodeScoreFunctor.h"

namespace form
{
	
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	
	class Mesh;
	class Shader;
	
	struct Quaterna;
	
	// contains a representation of a form::Scene concentrated on a (movable)
	// position such that detail is concentrated around that position; holds
	// information necessary to generate vertex and poly data such that polys will
	// be roughly the same screen size when viewed from given position
	class Surrounding
	{
		OBJECT_NO_COPY (Surrounding);
		
	public:
		
		// Constants
		enum {
			// derivations
			num_nodes_per_quaterna = 4,
			num_verts_per_quaterna = num_nodes_per_quaterna * 2,
			num_indices_per_quaterna = num_verts_per_quaterna * 3,
		};

		// Member functions
		Surrounding(size_t max_num_quaterne);
		~Surrounding();
		
#if defined(VERIFY)
		//void VerifyRanking(Ranking const * ranking) const;
		void Verify() const;
		void VerifyUsed(Quaterna const & q) const;
		void VerifyUnused(Quaterna const & q) const;
#endif
		PointBuffer & GetPoints() { return point_buffer; }
		
		std::size_t GetNumNodesUsed() const;
		std::size_t GetNumQuaternaUsed() const;
		std::size_t GetNumQuaternaUsedTarget() const;
		
		// returns 0 if there are none
		float GetMinParentScore() const;
		
		// returns max if there are none
		Scalar GetMinLeafDistanceSquared();
		
		// Must be a multiple of four.
		void SetNumQuaternaUsedTarget(std::size_t n);
		
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
		void ForEachQuaterna(FUNCTOR f, size_t step_size = 1, bool parallel = false);

		////////////////////////////////////////////////////////////////////////////////
		// variables

		NodeBuffer _node_buffer;
		
		// An array of used nodes in ascending order of score.
		Quaterna * const quaterne;		// [max_num_quaterne]

		Quaterna * quaterne_sorted_end;			// end of the range the we know is sorted
		Quaterna * quaterne_used_end;			// end of buffer of actually used quaterna
		Quaterna * quaterne_used_end_target;	// where we'd like quaterne_used_end to be; cannot be less than used_end
		Quaterna const * const quaterne_end;
		
		// Pool of vertices from which to take the corners of nodes.
		PointBuffer point_buffer;
		
		CalculateNodeScoreFunctor node_score_functor;
		Ray3 cached_node_score_ray;	// ray used when last the node buffer's scores were recalculated en masse. 
		
		// used by ChurnNodes
		SmpNodeVector _expandable_nodes;
	};
	
}
