//
//  Object.h
//  crag
//
//  Created by John McFarlane on 5/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once


////////////////////////////////////////////////////////////////////////////////
// Minimal definitions / declarations necessary 
// to declare a class with Python bindings.

namespace script
{
	template <typename CLASS> class MetaClass;
	
	class Object
	{
	public:
		PyObject_HEAD;
	};
}


#define DECLARE_SCRIPT_CLASS(CLASS, BASE_CLASS) \
	public: \
		typedef script::MetaClass<CLASS> MetaClass; \
	public: \
		friend class script::MetaClass<CLASS>; \
		static script::MetaClass<CLASS> _meta; \
		typedef BASE_CLASS super

#define DEFINE_SCRIPT_CLASS(NAMESPACE, CLASS) \
	script::MetaClass<NAMESPACE::CLASS> NAMESPACE::CLASS::_meta("crag." #CLASS)

