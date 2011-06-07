/*
 *  MetaClass.h
 *  crag
 *
 *  Created by John McFarlane on 1/31/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Object.h"

#include "core/Enumeration.h"


namespace script
{
	// TODO: Break out MetaClass and make use of Object 
	// to clean up how bases are dealt with.
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass helper declarations
	
	// IsVoid
	template <typename CLASS> bool IsVoid();

	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClassBase class
	
	// Note: This is the base class of MetaClass, 
	// Note: not the MetaClass for base classes.
	class MetaClassBase : public core::Enumeration<MetaClassBase>
	{
		typedef core::Enumeration<MetaClassBase> super;
	public:
		MetaClassBase(char const * name) 
		: super(name)
		{ 
		}
		virtual ~MetaClassBase()
		{
		}
		
		// TODO: Protect this by making InitModule a static member of the class.
		virtual void Init(PyObject & module, char const * module_name, char const * documentation) = 0;
		virtual void Deinit() = 0;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass class
	// 
	// Helps bind a native type, CLASS, to a Python class.
	
	template<typename CLASS> 
	class MetaClass : public MetaClassBase
	{
		OBJECT_SINGLETON(MetaClass);
		typedef typename CLASS::super BASE_CLASS;
		
	public:
		MetaClass(char const * name)
		: MetaClassBase(name)
		{
		}
		
		~MetaClass()
		{
			if (IsInitialized())
			{
				Deinit();
			}
		}
		
		bool IsInitialized() const
		{
			return _type_object.tp_name != nullptr;
		}
		
		static bool IsBaseClass()
		{
			return IsVoid<BASE_CLASS>();
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Cast functions
		//
		// Guide: Basically when casting, use reference versions 
		// when you're sure of the result. Otherwise, use pointers.
		
		// GetRef/Ptr functions convert a PyObject object into a CLASS object.
		static CLASS & GetRef(PyObject & self)
		{
			// Trying to cast to the wrong type of class?
			Assert (& _type_object == self.ob_type);

			// Cast from self to script::Object and then to derived type.
			Object & base_object = core::get_owner<Object, PyObject, & Object::ob_base>(self);
			CLASS & object = static_cast<CLASS &>(base_object);

			// Reverse the process to make sure everything's in the right place. 
			Assert(& object.ob_base == & self);
			Assert(object.ob_base.ob_type == & _type_object);

			return object;
		}
		static CLASS & GetRef(PyObject * self)
		{
			Assert (self != nullptr);
			return GetRef(* self);
		}
		
		static CLASS * GetPtr(PyObject & self)
		{
			if (& _type_object != self.ob_type)
			{
				return nullptr;
			}
			
			return & GetRef(self);
		}
		static CLASS * GetPtr(PyObject * self)
		{
			if (self == nullptr)
			{
				return nullptr;
			}
			
			return GetPtr(* self);
		}
		
	private:

		////////////////////////////////////////////////////////////////////////////////
		// MetaClassBase overrides
		
		virtual void Init(PyObject & module, char const * module_name, char const * documentation)
		{
			Assert(! IsInitialized());
			
			char const * name = GetName();
			char const * dot = strchr(name, '.');
			Assert(dot != nullptr);
			char const * class_name = dot + 1;
			
			_type_object.tp_name = name;
			_type_object.tp_basicsize = sizeof(CLASS);
			_type_object.tp_dealloc = DeleteObject;
			_type_object.tp_flags = IsBaseClass() ? Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE : Py_TPFLAGS_DEFAULT;
			_type_object.tp_doc = documentation;
			_type_object.tp_methods = _functions;
			
			_type_object.tp_base = IsBaseClass() ? nullptr : & MetaClass<BASE_CLASS>::_type_object;
			
			//_type_object.tp_init = reinterpret_cast<initproc>(Init);
			_type_object.tp_alloc = Alloc;
			_type_object.tp_new = NewObject;
			
			if (PyType_Ready(& _type_object) < 0)
			{
				Assert(false);
				return;
			}
			
			Py_INCREF(& _type_object);
			PyModule_AddObject(& module, class_name, (PyObject *)& _type_object);

			Assert(IsInitialized());
		}
		
		virtual void Deinit()
		{
			Assert(IsInitialized());
			
			// TODO: Clean up strings etc. from things like _type_object.tp_name?
			Py_DECREF(& _type_object);
			
			_type_object.tp_name = nullptr;

			Assert(! IsInitialized());			
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Python callbacks
		
		static PyObject * Alloc(PyTypeObject *self, Py_ssize_t nitems)
		{
			// Allocate the pre-constructed.
			char * memory = new char [sizeof(CLASS)];
			CLASS & uninitialized_object = * reinterpret_cast<CLASS *>(memory);
			
			// Initialize the memory (this is prior to a constructor being called).
			ZeroObject(uninitialized_object);
			uninitialized_object.ob_base.ob_refcnt = 1;
			uninitialized_object.ob_base.ob_type = & _type_object;

			// Note that the start of the allocation is NOT being returned.
			// Our objects need vtables so that's just not possible.
			return & uninitialized_object.ob_base;
		}
		
		static PyObject * NewObject(PyTypeObject * type, PyObject * args, PyObject * kwds)
		{
			PyObject * po = type->tp_alloc(type, 0);
			
			// Initialize its data member.
			if (po != nullptr) 
			{
				// Allocate the object.
				CLASS & object = GetRef(po);
				
				CLASS::Create(object, args); 
			}
			
			// Return the result.
			return po;
		}
		
		static void DeleteObject(PyObject * po)
		{
			CLASS & object = GetRef(po);
			CLASS::Destroy(object);
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// data
		
		static PyMethodDef _functions [];
	public:
		static PyTypeObject _type_object;		
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass member definitions
	
	template <typename CLASS>
	PyTypeObject MetaClass<CLASS>::_type_object = 
	{ 
		PyVarObject_HEAD_INIT(NULL, 0) 
	};

	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass helper definitions
	
	// IsVoid
	template <> inline bool IsVoid<void>()
	{
		return true;
	}
	template <typename CLASS> bool IsVoid()
	{
		return false;
	}
	
	// Casts
	template <typename CLASS> CLASS & GetRef(PyObject & self)
	{
		return MetaClass<CLASS>::GetRef(self);
	}	
	template <typename CLASS> CLASS & GetRef(PyObject * self)
	{
		return MetaClass<CLASS>::GetRef(self);
	}	
	
	template <typename CLASS> CLASS & GetPtr(PyObject & self)
	{
		return MetaClass<CLASS>::GetRef(self);
	}	
	template <typename CLASS> CLASS & GetPtr(PyObject * self)
	{
		return MetaClass<CLASS>::GetRef(self);
	}
	
}
