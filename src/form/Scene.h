//
//  Scene.h
//  crag
//
//  Created by John on 2/8/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/Polyhedron.h"

namespace geom
{
	class Space;
}

namespace gfx
{
	struct LodParameters;
}

namespace form
{
	class Formation;
	class Polyhedron;
	class Mesh;
	class Surrounding;
	
	// A representation (view?) of all the existing formations.
	// The scene is relative to a point in space which becomes the local space for the scene.
	// The reason for the class is that the coordinate space of even a single large formation
	// is too great to be represented using float coordinates and greater accuracy is required near the observer.
	class Scene
	{
		OBJECT_NO_COPY (Scene);
		
		// Types for keeping track of a formation using a model.
		typedef std::map<Formation const *, Polyhedron> FormationMap;
		typedef FormationMap::value_type FormationPair;
		
	public:
		
		///////////////////////////////////////////////////////
		// public interface
		
		Scene(size_t min_num_quaterne, size_t max_num_quaterne);
		~Scene();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Scene);

		void Clear();
		
		// Accessors for nodes/quaterna.
		Surrounding & GetSurrounding();
		Surrounding const & GetSurrounding() const;
		
		void OnSpaceReset(geom::Space const & space);
		bool IsSettled() const;

		void AddFormation(Formation & formation, geom::Space const & space);
		void RemoveFormation(Formation const & formation);
		Polyhedron const * GetPolyhedron(Formation const & formation) const;
		
		bool Tick(gfx::LodParameters const & lod_parameters);
		void GenerateMesh(Mesh & mesh, geom::Space const & space) const;
	private:
		
		///////////////////////////////////////////////////////
		// Formation-related members.
		
		void TickModels();
		void ResetPolyhedronSpaces(geom::Space const & space);
		void ResetFormations(geom::Space const & space);

		void TickPolyhedron(Polyhedron & model);

		void InitPolyhedron(FormationPair & pair, geom::Space const & space);
		void DeinitPolyhedron(FormationPair & pair);
		
		
		///////////////////////////////////////////////////////
		// Attributes
		
		Surrounding & _surrounding;
		
		FormationMap formation_map;		// The internal record of formations.
		
		bool _is_settled = false;
	};

}
