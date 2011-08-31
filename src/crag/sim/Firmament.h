/*
 *  Firmament.h
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Skybox.h"


class Firmament : public gfx::Skybox
{
private:
	virtual void Init();
	
	void DrawStarsClevur();

	void DrawStarsSimple();
	static void DrawStar(gfx::Image & side, Vector2f const & uv, float r);

	static const int box_edge_size = 512;
	static const int num_stars = 5000;
};
