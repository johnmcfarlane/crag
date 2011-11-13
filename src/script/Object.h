//
//  Object.h
//  crag
//
//  Created by John McFarlane on 5/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/intrusive_list.h"


////////////////////////////////////////////////////////////////////////////////
// Script class macros

#define DECLARE_SCRIPT_CLASS(CLASS, BASE_CLASS) \
	public: \
		static CLASS & GetRef(PyObject & self); \
		static CLASS & GetRef(PyObject * self); \
		static CLASS * GetPtr(PyObject & self); \
		static CLASS * GetPtr(PyObject * self); \
	private: \
		friend class script::MetaClass<CLASS>; \
		typedef script::MetaClass<CLASS> MetaClass; \
		typedef BASE_CLASS super; \
		static script::MetaClass<CLASS> _meta

#define DEFINE_SCRIPT_CLASS_BEGIN(NAMESPACE, CLASS) \
	NAMESPACE::CLASS & NAMESPACE::CLASS::GetRef(PyObject & self) { return script::GetRef<NAMESPACE::CLASS>(self); } \
	NAMESPACE::CLASS & NAMESPACE::CLASS::GetRef(PyObject * self) { return script::GetRef<NAMESPACE::CLASS>(self); } \
	NAMESPACE::CLASS * NAMESPACE::CLASS::GetPtr(PyObject & self) { return script::GetPtr<NAMESPACE::CLASS>(self); } \
	NAMESPACE::CLASS * NAMESPACE::CLASS::GetPtr(PyObject * self) { return script::GetPtr<NAMESPACE::CLASS>(self); } \
	script::MetaClass<NAMESPACE::CLASS> NAMESPACE::CLASS::_meta("crag." #CLASS); \
	namespace script { \
	template <> PyMethodDef script::MetaClassCommon<NAMESPACE::CLASS>::_methods[] = {

#define DEFINE_SCRIPT_CLASS_END \
	{NULL} }; }

#define DEFINE_SCRIPT_CLASS(NAMESPACE, CLASS) \
	DEFINE_SCRIPT_CLASS_BEGIN(NAMESPACE, CLASS) \
	DEFINE_SCRIPT_CLASS_END

#define SCRIPT_CLASS_METHOD(NAME, FUNCTION, DESCRIPTION) \
	{NAME, (PyCFunction)FUNCTION, METH_VARARGS, DESCRIPTION},


////////////////////////////////////////////////////////////////////////////////
// Minimal definitions / declarations necessary 
// to declare a class with Python bindings.

namespace script
{
	template <typename CLASS> class MetaClass;
	
	class Object
	{
		// types
		typedef core::intrusive::hook<Object> HookType;

	public:
		// variables
		PyObject_HEAD;
	private:
		HookType _hook;

		// more types
	public:
		DEFINE_INTRUSIVE_LIST_TYPE(Object, _hook, List);
		typedef List::const_iterator const_iterator;
		typedef List::iterator iterator;
	};
}
