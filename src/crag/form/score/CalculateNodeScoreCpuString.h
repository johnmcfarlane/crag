char const * kernel_source = "/*\n" \
" *  node_score.cg\n" \
" *  Crag\n" \
" *\n" \
" *  Created by John on 3/28/10.\n" \
" *  Copyright 2009, 2010 John McFarlane. All rights reserved.\n" \
" *  This program is distributed under the terms of the GNU General Public License.\n" \
" *\n" \
" */\n" \
"\n" \
"\n" \
"struct Triplet\n" \
"{\n" \
"	void * corner;\n" \
"	void * mid_point;\n" \
"	void * cousin;\n" \
"};\n" \
"\n" \
"\n" \
"struct Node\n" \
"{\n" \
"	void * children;	\n" \
"	void * parent;		\n" \
"	int seed;			\n" \
"\n" \
"	struct Triplet triple[3];	\n" \
"\n" \
"	float4 center_area;\n" \
"//	float center_x;\n" \
"//	float center_y;\n" \
"//	float center_z;\n" \
"//	float area;\n" \
"\n" \
"	float4 normal_score;\n" \
"//	float normal_x;\n" \
"//	float normal_y;\n" \
"//	float normal_z;\n" \
"//	float score;\n" \
"};\n" \
"\n" \
"\n" \
"kernel void score_nodes(\n" \
"   struct Node global * nodes,\n" \
"   unsigned int const count,\n" \
"   float4 const camera_position,\n" \
"   float4 const camera_direction)\n" \
"{\n" \
"	int i = get_global_id(0);\n" \
"	\n" \
"	// TODO: Find out which way is better: this or...\n" \
"/*	float4 center = nodes[i].center_area;\n" \
"	float4 normal = nodes[i].normal_score;\n" \
"	float score = center.w;	// area\n" \
"	center.w = 0;\n" \
"	normal.w = 0;*/\n" \
"\n" \
"	// ... this. \n" \
"	float4 center = \n" \
"	{ \n" \
"		nodes[i].center_area.x,\n" \
"		nodes[i].center_area.y,\n" \
"		nodes[i].center_area.z,\n" \
"		0\n" \
"	};\n" \
"	float4 normal = \n" \
"	{\n" \
"		nodes[i].normal_score.x,\n" \
"		nodes[i].normal_score.y,\n" \
"		nodes[i].normal_score.z,\n" \
"		0\n" \
"	};\n" \
"	float score = nodes[i].center_area.w;\n" \
"	\n" \
"	// Get distance and normalized vector between camera and node.\n" \
"	float4 camera_to_node = center - camera_position;\n" \
"	float distance = fast_length(camera_to_node);\n" \
"\n" \
"	// TODO: zero check?\n" \
"	//camera_to_node = normalize(camera_to_node);\n" \
"	camera_to_node *= native_recip(distance);\n" \
"\n" \
"	// As the poly turns away from the camera, its visible area diminishes.\n" \
"	// However, we still want invisible / barely visible polys to get some score.\n" \
"	float camera_dp = dot(camera_to_node, normal);\n" \
"//	float towardness_factor = max(camera_dp, 0.1f);\n" \
"//	score *= towardness_factor;\n" \
"	//score *= powr(5.f, camera_dp);\n" \
"	score *= exp(camera_dp);\n" \
"\n" \
"	// Distance-based falloff.\n" \
"	float fudged_min_visible_distance = .25f;	// TODO: hard-coded!\n" \
"	float clipped_distance = max(distance, fudged_min_visible_distance);\n" \
"	score *= native_recip(clipped_distance * clipped_distance);\n" \
"	\n" \
"	nodes[i].normal_score.w = score;\n" \
"}\n" \
"";
