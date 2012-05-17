//
//  Firmament.h
//  crag
//
//  Created by John on 11/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "gfx/object/Skybox.h"


DECLARE_CLASS_HANDLE(sim, Firmament);

namespace sim 
{
	class Firmament : public gfx::Skybox
	{
	public:
		Firmament(Init const & init);
		
	private:
		void DrawStarsClevur();
		
		void DrawStarsSimple();
		static void DrawStar(gfx::Image & side, geom::Vector2f const & uv, float r);
		
		static const int box_edge_size = 512;
		static const int num_stars = 20000;
	};
}
