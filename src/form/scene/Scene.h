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

#include "sim/defs.h"

#include <map>


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
		
		Scene();
		~Scene();
		
#if defined(VERIFY)
		void Verify() const;
		//void Verify(Mesh const & m) const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(Scene);
		
		void Clear();
		
		// Accessors for nodes/quaterna.
		NodeBuffer & GetNodeBuffer();
		NodeBuffer const & GetNodeBuffer() const;
		
		sim::Ray3 const & GetCameraRay() const;
		void SetCameraRay(sim::Ray3 const & camera_ray);
		
		sim::Vector3 const & GetOrigin() const;
		void SetOrigin(sim::Vector3 const & o);
		bool IsOriginOk() const;
		
		void AddFormation(Formation & formation);
		void RemoveFormation(Formation const & formation);
		Polyhedron const * GetPolyhedron(Formation const & formation) const;
		
		void Tick();
		void GenerateMesh(Mesh & mesh) const;
	private:
		
		///////////////////////////////////////////////////////
		// Formation-related members.
		
		void TickModels();
		void ResetPolyhedronOrigins();
		void ResetFormations();

		void TickPolyhedron(Polyhedron & model);

		void InitPolyhedron(FormationPair & pair);
		void DeinitPolyhedron(FormationPair & pair);
		
		
		///////////////////////////////////////////////////////
		// Attributes
		
		NodeBuffer * node_buffer;
		
		FormationMap formation_map;		// The internal record of formations.
		sim::Ray3 camera_ray;			// The observer position/direction in universal coordinates.
		sim::Ray3 camera_ray_relative;	// The observer position/direction relative to origin.
		sim::Vector3 origin;			// The zero point in universal coordinates.
	};

}
