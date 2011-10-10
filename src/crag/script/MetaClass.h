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
	// forward-declarations
	template <typename CLASS> CLASS & GetRef(PyObject & self);
	template <typename CLASS> CLASS & GetRef(PyObject * self);
	template <typename CLASS> CLASS * GetPtr(PyObject & self);
	template <typename CLASS> CLASS * GetPtr(PyObject * self);

	bool IsA(PyTypeObject const & lhs, PyTypeObject const & rhs);
	
	////////////////////////////////////////////////////////////////////////////////
	// MetaClassPoly class
	
	// Note: This is the base class of MetaClass, 
	// Note: not the MetaClass for base classes.
	class MetaClassPoly : public core::Enumeration<MetaClassPoly>
	{
		typedef core::Enumeration<MetaClassPoly> super;
	public:
		MetaClassPoly(char const * name);
		virtual ~MetaClassPoly();
		
		virtual bool Init(PyObject & module, char const * documentation) = 0;
		virtual void Deinit() = 0;

		// Given a brand new module, initialized it wrt all the classes.
		static void InitModule(PyObject & module);
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
		
		static void InitTypeObjectBase(PyTypeObject & derived_type_object)
		{
			derived_type_object.tp_base = & _type_object;
		}
		
	protected:
		
		static void InitTypeObject(PyTypeObject & type_object, char const * type_name, PyObject & module, char const * documentation)
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
		
		////////////////////////////////////////////////////////////////////////////////
		// MetaClassPoly overrides
		
		static bool ReadyAsBaseClass() 
		{
			return true;
		}

	private:
		
		virtual bool Init(PyObject & module, char const * documentation)
		{
			super::_type_object.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
			super::_type_object.tp_base = nullptr;
			
			InitTypeObject(super::_type_object, GetName(), module, documentation);
            
            return true;
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
		
		////////////////////////////////////////////////////////////////////////////////
		// MetaClassPoly overrides

		static bool ReadyAsBaseClass() 
		{
			return super::IsInitialized();
		}

	private:

		// Initialize this metaclass WRT the given module.
		bool Init(PyObject & module, char const * documentation) override
		{
			if (super::IsInitialized())
			{
				// This happens often as a result of the way InitModule operates.
				return true;
			}

			typedef MetaClass<BASE_CLASS> MetaBaseClass;
			if (! MetaBaseClass::ReadyAsBaseClass())
			{
				// Will have to wait until base class is done.
				return false;
			}

			MetaBaseClass::InitTypeObjectBase(super::_type_object);
			
			super::_type_object.tp_dealloc = DeallocObject;
			super::_type_object.tp_flags = Py_TPFLAGS_DEFAULT;
			super::_type_object.tp_init = InitObject;
			super::_type_object.tp_alloc = AllocObject;
			super::_type_object.tp_new = NewObject;

			super::InitTypeObject(super::_type_object, this->GetName(), module, documentation);

			return true;
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Python callbacks
		
		static int InitObject(PyObject * po, PyObject * args, PyObject * type)
		{
			// Allocate the object.
			CLASS & object = CLASS::GetRef(po);
			
			// Hold on to arguments object.
			// It will be put on a thread message queue.
			// (Decremented in sim::Simulation::OnMessage.)
			Py_INCREF(args);
			
			CLASS::Create(object, * args); 

			return 0;
		}

		// Return a pointer to enough memory to hold an instance of class, CLASS.
		static PyObject * AllocObject(PyTypeObject * type, Py_ssize_t nitems)
		{
			Assert(type == & super::_type_object);
			Assert(nitems == 0);
			Assert(type->tp_basicsize == sizeof(CLASS));

			// Allocate the pre-constructed.
			void * memory = Allocate(sizeof(CLASS));
			CLASS & uninitialized_object = * reinterpret_cast<CLASS *>(memory);
			
			// Initialize the memory (this is prior to a constructor being called).
			ZeroObject(uninitialized_object);
			PyObject_Init(& uninitialized_object.ob_base, & super::_type_object);
			
			// Note that the start of the allocation is NOT being returned.
			// Our objects need vtables so that's just not possible.
			return & uninitialized_object.ob_base;
		}
		
		// Allocate and construct a new instance of class, CLASS.
		static PyObject * NewObject(PyTypeObject * type, PyObject * args, PyObject * kwds)
		{
			PyObject * po = type->tp_alloc(type, 0);
			
			// Return the result.
			return po;
		}
		
		static void DeallocObject(PyObject * po)
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
	//
	// Guide: Basically when casting, use reference versions 
	// when you're sure of the result. Otherwise, use pointers.
	
	template <typename CLASS> CLASS & GetRef(PyObject & self)
	{
		// Trying to cast to the wrong type of class?
		Assert(IsA(* self.ob_type, MetaClass<CLASS>::_type_object));
		//Assert (& MetaClass<CLASS>::_type_object == self.ob_type);
		
		// Cast from self to script::Object and then to derived type.
		Object & base_object = core::get_owner<Object, PyObject, & Object::ob_base>(self);
		CLASS & object = static_cast<CLASS &>(base_object);
		
		// Reverse the process to make sure everything's in the right place. 
		Assert(& object.ob_base == & self);
		Assert(IsA(* object.ob_base.ob_type, MetaClass<CLASS>::_type_object));
		
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

	// returns true iff lhs is rhs - or is ultimately derived from rhs. 
	inline bool IsA(PyTypeObject const & lhs, PyTypeObject const & rhs)
	{
		if (& lhs == & rhs)
		{
			return true;
		}

		PyTypeObject const * lhs_base = lhs.tp_base;
		if (lhs_base == nullptr)
		{
			return false;
		}

		return IsA(* lhs_base, rhs);
	}
}
