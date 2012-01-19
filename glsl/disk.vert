#version 120

//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


////////////////////////////////////////////////////////////////////////////////
// outputs

varying vec4 quad_position;


////////////////////////////////////////////////////////////////////////////////
// functions




void main(void)
{
	quad_position = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
