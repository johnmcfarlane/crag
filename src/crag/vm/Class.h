/*
 *  Class.h
 *  crag
 *
 *  Created by John McFarlane on 1/31/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/Singleton.h"


namespace vm
{
	
	// Class class helps bind a native type, NativeType, to a Python class.
	template<typename NativeType> class Class : protected core::Singleton<Class <NativeType> >
	{
	public:
		Class(char const * qualified_name, char const * doc, PyObject * module)
		{
			char const * dot = strchr(qualified_name, '.');
			Assert(dot != nullptr);
			char const * name = dot + 1;
			
			type_object.tp_name = qualified_name;
			type_object.tp_basicsize = sizeof(Object);
			type_object.tp_dealloc = Dealloc;
			type_object.tp_flags = Py_TPFLAGS_DEFAULT;
			type_object.tp_doc = doc;
			type_object.tp_init = reinterpret_cast<initproc>(Init);
			type_object.tp_new = New;
			
			if (PyType_Ready(& type_object) < 0)
			{
				Assert(false);
				return;
			}
			
			Py_INCREF(& type_object);
			PyModule_AddObject(module, name, (PyObject *)& type_object);
		}
		
		~Class()
		{
			Py_DECREF(& type_object);
		}
		
	private:
		
		class Object
		{
		public:
			PyObject_HEAD
			NativeType * ptr;
		};
		
		static int Init(Object * o, PyObject * args, PyObject * kwds)
		{
			Assert(o != nullptr);
			
			NativeType * t = NativeType::Create(args); 
			if (t != nullptr)
			{
				o->ptr = t;
			}
			
			return 0;
		}
		
		static PyObject * New(PyTypeObject * type, PyObject * args, PyObject * kwds)
		{
			// Make sure we're talking about the right type of object.
			// (No reason to doubt this but for sanity's sake...)
			Assert(type == & type_object);
			
			// Allocate the object.
			Object * o = reinterpret_cast<Object *> (type->tp_alloc(type, 0));
			
			// Initialize its data member.
			if (o != nullptr) 
			{
				o->ptr = nullptr;
			}
			
			// Return the result.
			return (PyObject *)o;
		}
		
		static void Dealloc(PyObject * po)
		{
			Object * o = reinterpret_cast<Object *>(po);
			delete o->ptr;
		}
		
		// static data
		static PyTypeObject type_object;
	};
	
	
	template<typename NativeType> PyTypeObject Class<NativeType>::type_object = 
	{
		PyObject_HEAD_INIT(NULL)
	};

}
