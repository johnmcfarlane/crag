/*
 *  form/Scene.h
 *  Crag
 *
 *  Created by John on 2/8/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "NodeBuffer.h"
#include "Model.h"

#include "sim/defs.h"

#include <set>
#include <map>


class ProcessNodeFunctor;


namespace form
{
	typedef std::set<class Formation *> FormationSet;
	
	class FormationFunctor;
	class MeshBufferObject;
	
	// A representation (view?) of all the existing formations.
	// The scene is relative to a point in space which becomes the local origin for the scene.
	// The reason for the class is that the coordinate space of even a single large formation
	// is too great to be represented using float coordinates and greater accuracy is required near the observer.
	class Scene
	{
		friend class ::ProcessNodeFunctor;
		
		// Types for keeping track of a formation using a model.
		typedef std::map<Formation const *, Model> FormationMap;
		typedef FormationMap::value_type FormationPair;
		
	public:
		
		///////////////////////////////////////////////////////
		// public interface
		
		Scene();
		~Scene();
		
#if VERIFY
		void Verify() const;
		//void Verify(Mesh const & m) const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(Scene);
		
		void Clear();

		sim::Vector3 const & GetObserverPos() const;
		void SetObserverPos(sim::Vector3 const & pos, sim::Vector3 const & dir);
		
		sim::Vector3 const & GetOrigin() const;
		void SetOrigin(sim::Vector3 const & o);
		
		void Tick(FormationSet const & formation_set);
		void ForEachFormation(FormationFunctor & f) const;
		void GenerateMesh(class Mesh & mesh);
		
		// Accessors for nodes.
		int GetNumQuaternaUsed() const;
		int GetNumQuaternaUsedTarget() const;
		void SetNumQuaternaUsedTarget(int n);
		
		Model const & GetModel(Formation const & formation) const;
		
	private:
		
		///////////////////////////////////////////////////////
		// Formation-related members.
		
		void TickModels(FormationSet const & formation_set);
		void ResetFormations();

		void TickModel(Model & model);

		void InitModel(FormationPair & pair);
		void DeinitModel(FormationPair & pair);
		void ResetModel(FormationPair & pair);
		
		
		///////////////////////////////////////////////////////
		// Attributes
		
		NodeBuffer node_buffer;
		
		FormationMap formation_map;	// The internal record of formations.
		sim::Vector3 observer_pos;	// The observer position in universal coordinates.
		sim::Vector3 relative_observer_pos;	// The observer position relative to origin.
		sim::Vector3 observer_dir;	// The direction in which the observer is facing.
		sim::Vector3 origin;	// The zero point in universal coordinates.
	};

}
