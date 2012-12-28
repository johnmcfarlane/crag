//
//  Scene.h
//  crag
//
//  Created by John on 2/8/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/scene/Polyhedron.h"

#include "geom/origin.h"

namespace form
{
	class Formation;
	class Polyhedron;
	class Mesh;
	class NodeBuffer;
	
	// A representation (view?) of all the existing formations.
	// The scene is relative to a point in space which becomes the local origin for the scene.
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
		
#if defined(VERIFY)
		void Verify() const;
		//void Verify(Mesh const & m) const;
#endif
		void Clear();
		
		// Accessors for nodes/quaterna.
		NodeBuffer & GetNodeBuffer();
		NodeBuffer const & GetNodeBuffer() const;
		
		void OnOriginReset(geom::abs::Vector3 const & origin);

		void AddFormation(Formation & formation, geom::abs::Vector3 const & origin);
		void RemoveFormation(Formation const & formation);
		Polyhedron const * GetPolyhedron(Formation const & formation) const;
		
		void Tick(geom::rel::Ray3 const & camera_ray);
		void GenerateMesh(Mesh & mesh, geom::abs::Vector3 const & origin) const;
	private:
		
		///////////////////////////////////////////////////////
		// Formation-related members.
		
		void TickModels();
		void ResetPolyhedronOrigins(geom::abs::Vector3 const & origin);
		void ResetFormations(geom::abs::Vector3 const & origin);

		void TickPolyhedron(Polyhedron & model);

		void InitPolyhedron(FormationPair & pair, geom::abs::Vector3 const & origin);
		void DeinitPolyhedron(FormationPair & pair);
		
		
		///////////////////////////////////////////////////////
		// Attributes
		
		NodeBuffer & _node_buffer;
		
		FormationMap formation_map;		// The internal record of formations.
	};

}
