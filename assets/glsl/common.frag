//
//  common.frag
//  crag
//
//  Created by John McFarlane on 2014-08-25.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(GL_ES)

#if defined(GL_FRAGMENT_PRECISION_HIGH)
#define VECTOR_PRECISION highp
precision highp float;
precision highp int;
#else
#define VECTOR_PRECISION mediump
precision mediump float;
precision mediump int;
#endif

#endif
