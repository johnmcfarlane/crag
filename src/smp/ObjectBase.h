//
//  ObjectBase.h
//  crag
//
//  Created by John McFarlane on 2012-11-26.
//	Copyright 2009-2012 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace smp
{
	// This class serves purely as a workaround for unexplained run-time behavior 
	// which only occurs in WIN32 builds.
	template <typename OBJECT>
	class ObjectBase
	{
		OBJECT_NO_COPY(ObjectBase);

	public:
		// types
		typedef OBJECT ObjectType;

		// functions
		ObjectBase()
		{
		}

		operator ObjectType & ()
		{
			ObjectType & t = * static_cast<ObjectType *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
		operator ObjectType const & () const
		{
			ObjectType const & t = * static_cast<ObjectType const *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase const & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}

	protected:
		~ObjectBase() { }

	private:
		DEFINE_TEMPLATED_INTRUSIVE_LIST(ObjectBase, List);
	};
}
