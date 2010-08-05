/*
 *  node_score.cg
 *  Crag
 *
 *  Created by John on 3/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


struct Triplet
{
	void * corner;
	void * mid_point;
	void * cousin;
};


struct Node
{
	void * children;	
	void * parent;		
	int seed;			

	struct Triplet triple[3];	

	float4 center_area;
//	float center_x;
//	float center_y;
//	float center_z;
//	float area;

	float4 normal_score;
//	float normal_x;
//	float normal_y;
//	float normal_z;
//	float score;
};


kernel void score_nodes(
   struct Node global * nodes,
   unsigned int const count,
   float4 const camera_position,
   float4 const camera_direction)
{
	int i = get_global_id(0);
	
	// TODO: Find out which way is better: this or...
	float4 center = nodes[i].center_area;
	float4 normal = nodes[i].normal_score;
	float score = center.w;	// area
	center.w = 0;
	normal.w = 0;

	// ... this. 
	/*float4 center = 
	{ 
		nodes[i].center_area.x,
		nodes[i].center_area.y,
		nodes[i].center_area.z,
		0
	};
	float4 normal = 
	{
		nodes[i].normal_score.x,
		nodes[i].normal_score.y,
		nodes[i].normal_score.z,
		0
	};
	float score = nodes[i].center_area.w;*/
	
	// Get distance and normalized vector between camera and node.
	float4 camera_to_node = center - camera_position;
	float distance_squared = dot(camera_to_node, camera_to_node);

	// TODO: zero check?
	//camera_to_node = normalize(camera_to_node);
	camera_to_node *= rsqrt(distance_squared);

	// As the poly turns away from the camera, its visible area diminishes.
	// However, we still want invisible / barely visible polys to get some score.
	float camera_dp = dot(camera_to_node, normal);
	float towardness_factor = max(camera_dp, 0.1f);
	score *= towardness_factor;

	// Distance-based falloff.
	float fudged_min_visible_distance = .25f;	// TODO: hard-coded!
	score /= max(distance_squared, fudged_min_visible_distance * fudged_min_visible_distance);
	
	nodes[i].normal_score.w = score;
}
