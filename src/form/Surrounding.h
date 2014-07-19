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
#include "QuaternaBuffer.h"
#include "NodeBuffer.h"

#include "gfx/LodParameters.h"

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
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DECLARE(Surrounding);
		void VerifyUsed(Quaterna const & q) const;
		void VerifyUnused(Quaterna const & q) const;
#endif

		PointBuffer const & GetPoints() const;
		PointBuffer & GetPoints();
		
		int GetNumNodesUsed() const;
		int GetNumQuaternaUsed() const;
		
		// returns 0 if there are none
		float GetMinParentScore() const;
		
		// returns max if there are none
		Scalar GetMinLeafDistanceSquared() const;
		
		// Must be a multiple of four.
		int GetTargetNumQuaterna() const;
		void SetTargetNumQuaterna(int n);
		
		bool Tick(gfx::LodParameters const & lod_parameters);
		void OnReset();
		void ResetNodeOrigins(Vector3 const & origin_delta);
	private:
		void InitQuaterna(Quaterna const * end);
		
		void UpdateNodes();
		void UpdateNodeScores(gfx::LodParameters const & lod_parameters);
		void UpdateQuaterna();
		void ExpandNodes();
	public:
		
		void ResetMeshPointers();
		void GenerateMesh(Mesh & mesh);
		
		bool IsChildNode(Node const & node) const;
		bool IsValidNodePointer(Node const * node) const;
		int GetNodeIndex(Node const & node) const;
		
		///////////////////////////////////////////////////////
		// Node-related members.
		static bool IsNodeChurnIntensive() { return true; }
		float GetLowestSortedQuaternaScore() const;
		bool ExpandNode(Node & node);
		bool ExpandNode(Node & node, Quaterna & children_quaterna);
		void CollapseNodes(Node & root);
	private:
		void CollapseNode(Node & node);
		static bool InitChildGeometry(Node const & parent, Node * children_copy);
		static void InitChildPointers(Node & parent_node);
		
		void DeinitChildren(Node * children);
		void DeinitNode(Node & node);
		
		void IncreaseNodes(int target_num_quaterne);
		void DecreaseNodes(int target_num_quaterne);
		
		void DecreaseQuaterna(int new_num_quaterne);
		void FixUpDecreasedNodes(int old_num_quaterne);

		////////////////////////////////////////////////////////////////////////////////
		// variables

		NodeBuffer _node_buffer;
		
		QuaternaBuffer _quaterna_buffer;
		int _target_num_quaterne;
		// Pool of vertices from which to take the corners of nodes.
		PointBuffer point_buffer;
		
		CalculateNodeScoreFunctor node_score_functor;
		
		// used by ChurnNodes
		SmpNodeVector _expandable_nodes;
		
		bool _changed;
	};
	
}
