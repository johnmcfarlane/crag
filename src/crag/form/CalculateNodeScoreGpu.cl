/*
 *  node_score.cg
 *  Crag
 *
 *  Created by John on 3/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


struct Input
{
	float center_x;
	float center_y;
	float center_z;
	float area;
};


__kernel void score_nodes(
	global struct Input * nodes,
	global float * scores,
	const unsigned int count,
	const float4 camera_position)
{
	int i = get_global_id(0);
	
	float score;

	score = nodes[i].area;

	// distance
	float4 center = 
	{
		nodes[i].center_x,
		nodes[i].center_y,
		nodes[i].center_z,
		0
	};
	float4 to_camera = camera_position - center;
	
	// TODO: Is this optimal way to length squared?
	float distance_squared = dot(to_camera, to_camera);

	// TODO: zero check?
	to_camera = normalize(to_camera);

	// Distance-based falloff.
	float fudged_min_visible_distance = 1;	// TODO: hard-coded!
	score /= max(distance_squared, fudged_min_visible_distance * fudged_min_visible_distance);
	
	scores[i] = score;
}
