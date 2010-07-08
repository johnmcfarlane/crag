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
"struct Input\n" \
"{\n" \
"	float center_x;\n" \
"	float center_y;\n" \
"	float center_z;\n" \
"	float area;\n" \
"};\n" \
"\n" \
"\n" \
"__kernel void score_nodes(\n" \
"	global struct Input * nodes,\n" \
"	global float * scores,\n" \
"	const unsigned int count,\n" \
"	const float4 camera_position)\n" \
"{\n" \
"	int i = get_global_id(0);\n" \
"	\n" \
"	float score;\n" \
"\n" \
"	score = nodes[i].area;\n" \
"\n" \
"	// distance\n" \
"	float4 center = \n" \
"	{\n" \
"		nodes[i].center_x,\n" \
"		nodes[i].center_y,\n" \
"		nodes[i].center_z,\n" \
"		0\n" \
"	};\n" \
"	float4 to_camera = camera_position - center;\n" \
"	\n" \
"	// TODO: Is this optimal way to length squared?\n" \
"	float distance_squared = dot(to_camera, to_camera);\n" \
"\n" \
"	// TODO: zero check?\n" \
"	to_camera = normalize(to_camera);\n" \
"\n" \
"	// Distance-based falloff.\n" \
"	float fudged_min_visible_distance = 1;	// TODO: hard-coded!\n" \
"	score /= max(distance_squared, fudged_min_visible_distance * fudged_min_visible_distance);\n" \
"	\n" \
"	scores[i] = score;\n" \
"}\n" \
"";