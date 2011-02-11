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


namespace script
{
	
	// Class class helps bind a native type, NativeType, to a Python class.
	template<typename NativeType> 
	class Class : protected core::Singleton<Class <NativeType> >
	{
	public:
		template <typename BaseClass>
		Class(char const * qualified_name, PyObject & module, char const * doc, BaseClass & base_class)
		{
			InitType(qualified_name, module, doc, & base_class.GetTypeObject());
		}
		
		Class(char const * qualified_name, PyObject & module, char const * doc)
		{
			InitType(qualified_name, module, doc, nullptr);
		}
		
		~Class()
		{
			Py_DECREF(& type_object);
		}
		
		PyTypeObject & GetTypeObject() 
		{
			return type_object;
		}
		
	private:
		
		class Object
		{
		public:
			PyObject_HEAD
			NativeType * ptr;
		};
		
		void InitType(char const * qualified_name, PyObject & module, char const * doc, PyTypeObject * base_type)
		{
			PyTypeObject blank_type_object = 
			{
				PyObject_HEAD_INIT(NULL)
			};
			memcpy(& type_object, & blank_type_object, sizeof(PyTypeObject));
			
			char const * dot = strchr(qualified_name, '.');
			Assert(dot != nullptr);
			char const * name = dot + 1;
			
			type_object.tp_name = qualified_name;
			type_object.tp_basicsize = sizeof(Object);
			type_object.tp_dealloc = Dealloc;
			type_object.tp_flags = (base_type == nullptr) ? Py_TPFLAGS_DEFAULT : Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
			type_object.tp_doc = doc;
			type_object.tp_base = base_type;
			type_object.tp_init = reinterpret_cast<initproc>(Init);
			type_object.tp_new = New;
			
			if (PyType_Ready(& type_object) < 0)
			{
				Assert(false);
				return;
			}
			
			Py_INCREF(& type_object);
			PyModule_AddObject(& module, name, (PyObject *)& type_object);
		}
		
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
		PyTypeObject type_object;
	};

}
