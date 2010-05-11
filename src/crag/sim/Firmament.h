/*
 *  Firmament.h
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "gfx/Skybox.h"


class Firmament : public gfx::Skybox
{
public:
	Firmament();
	
	static void DrawStar(gfx::Image & side, Vector2f const & uv, float r);
	
private:
	static const int box_edge_size = 256; 
};
