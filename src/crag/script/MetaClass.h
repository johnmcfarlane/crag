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
	template <typename CLASS> CLASS & GetRef(PyObject & self);
	template <typename CLASS> CLASS & GetRef(PyObject * self);
	template <typename CLASS> CLASS * GetPtr(PyObject & self);
	template <typename CLASS> CLASS * GetPtr(PyObject * self);
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClassPoly class
	
	// Note: This is the base class of MetaClass, 
	// Note: not the MetaClass for base classes.
	class MetaClassPoly : public core::Enumeration<MetaClassPoly>
	{
		typedef core::Enumeration<MetaClassPoly> super;
	public:
		MetaClassPoly(char const * name) 
		: super(name)
		{ 
		}
		virtual ~MetaClassPoly()
		{
		}
		
		virtual void Init(PyObject & module, char const * module_name, char const * documentation) = 0;
		virtual void Deinit() = 0;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClassCommon class
	// 
	// Templated members common to all MetaClass specializations.
	
	template <typename CLASS>
	class MetaClassCommon : public MetaClassPoly
	{
		friend CLASS & GetRef<CLASS>(PyObject & self);
		friend CLASS * GetPtr<CLASS>(PyObject & self);
		
		OBJECT_SINGLETON(MetaClassCommon);
		
	public:
		MetaClassCommon(char const * name)
		: MetaClassPoly(name)
		{
		}
		
		~MetaClassCommon()
		{
			if (IsInitialized())
			{
				Deinit();
			}
		}
		
		static bool IsInitialized() 
		{
			return _type_object.tp_name != nullptr;
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Cast functions
		//
		// Guide: Basically when casting, use reference versions 
		// when you're sure of the result. Otherwise, use pointers.
		
		static void InitTypeObjectBase(PyTypeObject & type_object)
		{
			type_object.tp_base = & _type_object;
		}
		
	protected:
		
		static void InitTypeObject(PyTypeObject & type_object, char const * type_name, PyObject & module, char const * module_name, char const * documentation)
		{
			Assert(! IsInitialized());
			
			char const * name = type_name;
			char const * dot = strchr(name, '.');
			Assert(dot != nullptr);
			char const * class_name = dot + 1;
			
			_type_object.tp_name = name;
			_type_object.tp_basicsize = sizeof(CLASS);
			_type_object.tp_doc = documentation;
			_type_object.tp_methods = _methods;
			
			if (PyType_Ready(& _type_object) < 0)
			{
				Assert(false);
				return;
			}
			
			Py_INCREF(& _type_object);
			PyModule_AddObject(& module, class_name, (PyObject *)& _type_object);
			
			Assert(IsInitialized());
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// MetaClassPoly overrides
		
		virtual void Deinit()
		{
			Assert(IsInitialized());
			
			Py_DECREF(& _type_object);			
			_type_object.tp_name = nullptr;
			
			Assert(! IsInitialized());			
		}

		////////////////////////////////////////////////////////////////////////////////
		// data
		
		static PyMethodDef _methods [];
		static PyTypeObject _type_object;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass class
	// 
	// Helps bind a native type, CLASS, to a Python class.
	// Has two specializations: root and derived.
	
	template<typename CLASS> class MetaClass;
	
	
	// root (or top) class specialization of MetaClass
	template<> 
	class MetaClass<Object> : public MetaClassCommon<Object>
	{
		typedef MetaClassCommon<Object> super;
	public:
		MetaClass(char const * name)
		: super(name)
		{
		}
		
	private:
		
		////////////////////////////////////////////////////////////////////////////////
		// MetaClassPoly overrides
		
		virtual void Init(PyObject & module, char const * module_name, char const * documentation)
		{
			super::_type_object.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
			super::_type_object.tp_base = nullptr;
			
			InitTypeObject(super::_type_object, GetName(), module, module_name, documentation);
		}
	};
	
	
	// non-root class specialization of MetaClass
	template<typename CLASS> 
	class MetaClass : public MetaClassCommon<CLASS>
	{
		typedef MetaClassCommon<CLASS> super;
		typedef typename CLASS::super BASE_CLASS;
	public:
		MetaClass(char const * name)
		: super(name)
		{
		}
		
	private:

		////////////////////////////////////////////////////////////////////////////////
		// MetaClassPoly overrides
		
		virtual void Init(PyObject & module, char const * module_name, char const * documentation)
		{
			MetaClass<BASE_CLASS>::InitTypeObjectBase(super::_type_object);
			
			super::_type_object.tp_dealloc = DeleteObject;
			super::_type_object.tp_flags = Py_TPFLAGS_DEFAULT;
			super::_type_object.tp_alloc = Alloc;
			super::_type_object.tp_new = NewObject;

			super::InitTypeObject(super::_type_object, this->GetName(), module, module_name, documentation);
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
			uninitialized_object.ob_base.ob_type = & super::_type_object;
			
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
				CLASS & object = CLASS::GetRef(po);
				
				CLASS::Create(object, * args); 
			}
			
			// Return the result.
			return po;
		}
		
		static void DeleteObject(PyObject * po)
		{
			CLASS & object = CLASS::GetRef(po);
			CLASS::Destroy(object);
		}
		
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClass member definitions
	
	template <typename CLASS>
	PyTypeObject MetaClassCommon<CLASS>::_type_object = 
	{ 
		PyVarObject_HEAD_INIT(NULL, 0) 
	};

	
	////////////////////////////////////////////////////////////////////////////////
	// Functions to cast from PyObject to native objects.
	
	template <typename CLASS> CLASS & GetRef(PyObject & self)
	{
		// Trying to cast to the wrong type of class?
		Assert (& MetaClass<CLASS>::_type_object == self.ob_type);
		
		// Cast from self to script::Object and then to derived type.
		Object & base_object = core::get_owner<Object, PyObject, & Object::ob_base>(self);
		CLASS & object = static_cast<CLASS &>(base_object);
		
		// Reverse the process to make sure everything's in the right place. 
		Assert(& object.ob_base == & self);
		Assert(object.ob_base.ob_type == & MetaClass<CLASS>::_type_object);
		
		return object;
	}
	
	template <typename CLASS> CLASS & GetRef(PyObject * self)
	{
		Assert (self != nullptr);
		return GetRef<CLASS>(* self);
	}
	
	template <typename CLASS> CLASS * GetPtr(PyObject & self)
	{
		if (& MetaClass<CLASS>::_type_object != self.ob_type)
		{
			return nullptr;
		}
		
		return & GetRef<CLASS>(self);
	}
	
	template <typename CLASS> CLASS * GetPtr(PyObject * self)
	{
		if (self == nullptr)
		{
			return nullptr;
		}
		
		return GetPtr<CLASS>(* self);
	}
	
}
