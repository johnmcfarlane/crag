//
//  Object.cpp
//  crag
//
//  Created by john on 2014-07-05.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Object.h"

#include "Engine.h"

using namespace form;

Object::Object(Engine & engine, Formation formation)
: super(engine)
, _formation(formation)
{
	GetEngine().OnAddFormation(_formation);
}

Object::~Object()
{
	GetEngine().OnRemoveFormation(_formation);
}
