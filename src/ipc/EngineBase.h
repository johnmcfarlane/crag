//
//  EngineBase.h
//  crag
//
//  Created by John on 2012/12/23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ObjectBase.h"

namespace ipc
{
	// an optional base class for the Engine classes which are managed by Daemon
	// with support for object lifetime management
	template <typename ENGINE, typename OBJECT>
	class EngineBase
	{
		OBJECT_NO_COPY(EngineBase);
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ENGINE Engine;
		typedef OBJECT Object;
		typedef ipc::ObjectBase<Object, ENGINE> ObjectBase;
		typedef std::unordered_map<Uid, ObjectBase *> ObjectMap;
		typedef typename ObjectMap::iterator Iterator;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		EngineBase()
			: _objects()
		{
		}

		virtual ~EngineBase()
		{
			if (! IsEmpty())
			{
				DEBUG_BREAK(SIZE_T_FORMAT_SPEC " object(s) remaining", _objects.size());
			}
		}

		bool IsEmpty() const
		{
			return _objects.empty();
		}

		// object management
		Object * GetObject(Uid uid)
		{
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				return nullptr;
			}

			ObjectBase & object_base = ref(found->second);
			Object & object = object_base;
			return & object;
		}

		Object const * GetObject(Uid uid) const
		{
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				return nullptr;
			}

			ObjectBase & object_base = ref(found->second);
			Object & object = object_base;
			return & object;
		}

		// for each map pair
		template <typename FUNCTION>
		void ForEachPair(FUNCTION f)
		{
			for (auto pair : _objects)
			{
				f(pair);
			}
		}

		// for each map pair
		template <typename FUNCTION>
		void ForEachPair(FUNCTION f) const
		{
			for (auto pair : _objects)
			{
				f(pair);
			}
		}

		// for each object
		template <typename FUNCTION>
		void ForEachObject(FUNCTION f)
		{
			ForEachPair([& f] (std::pair<Uid, ObjectBase *> pair) {
				f(ref(pair.second));
			});
		}

		template <typename FUNCTION>
		void ForEachObject(FUNCTION f) const
		{
			ForEachPair([& f] (std::pair<Uid, ObjectBase *> pair) {
				f(ref(pair.second));
			});
		}

		// for each object (f returns false if it is to be erased)
		template <typename FUNCTION>
		void ForEachObject_DestroyIf(FUNCTION f)
		{
			for (auto i = _objects.begin(); i != _objects.end();)
			{
				auto pair = * i;
				if (f(* pair.second))
				{
					i = DestroyObject(i);
				}
				else
				{
					++ i;
				}
			}
		}

		template <typename OBJECT_TYPE>
		OBJECT_TYPE * CreateObject()
		{
			return CreateObject<OBJECT_TYPE>(Uid::Create());
		}
		
#if defined(WIN32)
		template <typename OBJECT_TYPE>
		OBJECT_TYPE * CreateObject(Uid uid)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE();

			if (object != nullptr)
			{
				AddObject(uid, * object);
			}
			
			return object;
		}
#endif

		template <typename OBJECT_TYPE, typename ... PARAMETERS>
		OBJECT_TYPE * CreateObject(Uid uid, PARAMETERS const & ... parameters)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE(core::StaticCast<Engine>(* this), parameters ...);

			if (object != nullptr)
			{
				AddObject(uid, * object);
			}
			
			return object;
		}

		void DestroyObject(Uid uid)
		{
			ASSERT(uid);
			
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				DEBUG_MESSAGE("Object not found");
				return;
			}

			DestroyObject(found);
		}

		Iterator DestroyObject(Iterator destroyed)
		{
			auto object = destroyed->second;
			OnRemoveObject(* object);
			auto next = _objects.erase(destroyed);
			delete object;

			return next;
		}

	private:
		virtual void OnAddObject(Object &) { }
		virtual void OnRemoveObject(Object &) { }

		void AddObject(Uid uid, Object & object)
		{
			CRAG_VERIFY_TRUE(uid);
			ASSERT(! GetObject(uid));

			CRAG_VERIFY(object);
			CRAG_VERIFY_TRUE(! object.GetUid());

			object.SetUid(uid);

			_objects[uid] = & object;
			OnAddObject(object);
		}
		
	public:

		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(EngineBase, self)
			self.ForEachPair([& self] (std::pair<Uid, ObjectBase *> const & pair) {
				ObjectBase const & object_base = ref(pair.second);
				CRAG_VERIFY_EQUAL(pair.first, object_base.GetUid());
				CRAG_VERIFY_EQUAL(& self, & object_base.GetEngine());
				CRAG_VERIFY(object_base);
			});
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		////////////////////////////////////////////////////////////////////////////////
		// variables

	private:
		ObjectMap _objects;
	};
}
