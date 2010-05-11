/*
 *  Firmament.h
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
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
