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

#include "sim/axes.h"


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
		
		axes::RayRel const & GetCameraRay() const;
		void SetCameraRay(axes::RayRel const & camera_ray);
		void SetCameraRay(axes::RayAbs const & camera_ray);
		
		axes::VectorAbs const & GetOrigin() const;
		void SetOrigin(axes::VectorAbs const & o);
		
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
		
		NodeBuffer & _node_buffer;
		
		FormationMap formation_map;		// The internal record of formations.
		axes::RayAbs camera_ray;			// The observer position/direction in universal coordinates.
		axes::RayRel camera_ray_relative;	// The observer position/direction relative to origin.
		axes::VectorAbs origin;			// The zero point in universal coordinates.
	};

}
