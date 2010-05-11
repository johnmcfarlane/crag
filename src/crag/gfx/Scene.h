/*
 *  Scene.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Pov.h"
#include "sim/Defs.h"
#include "ShadowMap.h"

#include <map>
#include <vector>


namespace sim
{
	class Entity;
}


namespace gfx
{
	class Light;
	class Skybox;
	class ShadowMapKey;
	class ShadowMap;

	typedef std::map<gfx::ShadowMapKey, gfx::ShadowMap> ShadowMapMap;


	class Scene
	{
		friend class Renderer;
		
	public:
		Scene();
		~Scene();
		
		void AddLight(Light const & light);
		void AddEntity(sim::Entity const & entity);
		
		void SetResolution(Vector2i const & r);
		void SetCamera(sim::Vector3 const & pos, sim::Matrix4 const & rot);
		Pov & GetPov();
		
		void SetSkybox(Skybox const * s) { skybox = s; }
		Skybox const * GetSkybox() const { return skybox; }
		
	private:
		Pov pov;

		std::vector<Light const *> lights;
		std::vector<sim::Entity const *> entities;
		ShadowMapMap shadow_maps;
		Skybox const * skybox;
	};
}
