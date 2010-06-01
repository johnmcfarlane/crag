/*
 *  Physics.h
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/Singleton.h"

#include "form/Mesh.h"

#include "sim/defs.h"

#include <ode/ode.h>


namespace sim
{
	class Entity;
	typedef Entity UserDataType;


	//////////////////////////////////////////////////////////////////////
	// dBodyID support functions

	Vector3 const & GetPosition(dBodyID body);
	void SetPosition(dBodyID body, sim::Vector3 const & pos);

	void GetRotation(dBodyID body, sim::Matrix4 & rot);	// returns 12 floats
	void SetRotation(dBodyID body, sim::Matrix4 const & rot);

	void AddRelTorque(dBodyID body, sim::Vector3 const & v);
	void AddRelForce(dBodyID body, sim::Vector3 const & v);
	void AddRelForceAtRelPos(dBodyID body, sim::Vector3 const & force, sim::Vector3 const & pos);

	void SetMass (dBodyID body, const dMass *mass);
	void SetDamping(dBodyID body, float linear, float angular);


	//////////////////////////////////////////////////////////////////////
	// dGeomID support functions
	
	Vector3 const & GetPosition(dGeomID geom);
	void SetPosition(dGeomID geom, sim::Vector3 const & pos);
	
	
	//////////////////////////////////////////////////////////////////////
	// Physics class
	
	class Physics : public core::Singleton<Physics>
	{
		typedef std::vector<class PhysicalBody *> PhysicalBodyContainer;
	public:
		Physics();
		~Physics();

		form::Mesh & GetFormationMesh();
	
		dBodyID CreateBody(UserDataType & data);
		static void Destroy(dBodyID body);
		static UserDataType & GetUserData(dBodyID geom);

		dGeomID CreateSphere(UserDataType & data, float density, float radius);
		static void Destroy(dGeomID geom);
		static UserDataType & GetUserData(dGeomID geom);

		void AddPhysicalBody(PhysicalBody & pb);
		void RemovePhysicalBody(PhysicalBody & pb);
		bool ContainsPhysicalBody(PhysicalBody const & pb) const;

		void Tick(double delta_time);
	private:
		void CreateCollisions();
		void DestroyCollisions();

	public:
		void OnMeshCollision(dGeomID geom);

		void OnCollision(dGeomID geom1, dGeomID geom2);
		void OnContact(dContact * contact, dGeomID geom1, dGeomID geom2);
	
		static void DrawMeshData(dGeomID formation_geom, int num_triangles);

	private:
		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;
		PhysicalBodyContainer physical_bodies;
	
		// The portion of the formations with which it has close proximity.
		//form::Mesh formation_mesh;
		dTriMeshDataID mesh_data;
		dGeomID formation_geom;
	};
}
