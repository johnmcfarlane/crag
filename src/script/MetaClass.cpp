//
//  script/MetaClass.cpp
//  crag
//
//  Created by John McFarlane on 2011/10/9.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MetaClass.h"


using namespace script;


MetaClassPoly::MetaClassPoly(char const * name) 
: super(name)
{ 
}

MetaClassPoly::~MetaClassPoly()
{
}

void MetaClassPoly::InitModule(PyObject & module)
{
	iterator meta_classes_begin = begin();
	iterator meta_classes_end = end();

	// The classes are enumerated in arbitrary order
	// but it's probably wise to initialize base classes first. (TODO: Verify this.)
	// So we keep iterating through and Init returns false for 
	// classes whose base is not yet initialized. 
	// This isn't very efficient but it's simple and
	// there are few classes and little depth.
	bool done;
	do
	{
		done = true;
		for (iterator i = meta_classes_begin; i != meta_classes_end; ++ i)
		{
			script::MetaClassPoly & meta_class = * i;
			if (! meta_class.Init(module, "No documentation available."))	// TODO: Documentation
			{
				// This class couldn't be initialized because its base wasn't initialized yet.
				// It may be later in the list. Next time around, it should be ok.
				done = false;
			}
		}
	}	while (! done);
}
