//
//  EngineBase.h
//  crag
//
//  Created by John on 2012/12/23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/origin.h"

namespace smp
{
	// an optional base class for the Engine classes which are managed by Daemon
	// with support for local coordinate origins and object lifetime management
	template <typename ENGINE, typename OBJECT>
	class EngineBase
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ENGINE Engine;
		typedef OBJECT Object;
		typedef smp::Object<Object, ENGINE> SmpObject;
		typedef std::unordered_map<Uid, SmpObject *> ObjectMap;
		typedef typename ObjectMap::iterator Iterator;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		EngineBase()
			: _origin(geom::abs::Vector3::Zero())
		{
		}

		~EngineBase()
		{
			ASSERT(IsEmpty());
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

			SmpObject & smp_object = ref(found->second);
			Object & object = smp_object;
			return & object;
		}

		Object const * GetObject(Uid uid) const
		{
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				return nullptr;
			}

			SmpObject & smp_object = ref(found->second);
			Object & object = smp_object;
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
			ForEachPair([& f] (std::pair<Uid, SmpObject *> pair) {
				f(ref(pair.second));
			});
		}

		template <typename FUNCTION>
		void ForEachObject(FUNCTION f) const
		{
			ForEachPair([& f] (std::pair<Uid, SmpObject *> pair) {
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

#if defined(WIN32)
		template <typename OBJECT_TYPE>
		void CreateObject(Uid uid)
		{
			smp::ObjectInit<Engine>
			init = 
			{
				static_cast<Engine &>(* this),
				uid
			};
			OBJECT_TYPE * object = new OBJECT_TYPE(init);

			if (object != nullptr)
			{
				AddObject(* object);
			}
		}
#endif

		template <typename OBJECT_TYPE, typename ... PARAMETERS>
		void CreateObject(Uid uid, PARAMETERS const & ... parameters)
		{
			smp::ObjectInit<Engine>
			init = 
			{
				static_cast<Engine &>(* this),
				uid
			};
			OBJECT_TYPE * object = new OBJECT_TYPE(init, parameters ...);

			if (object != nullptr)
			{
				AddObject(* object);
			}
		}

		void DestroyObject(Uid uid)
		{
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
		virtual void OnAddObject(Object & object) { }
		virtual void OnRemoveObject(Object & object) { }

		void AddObject(Object & object)
		{
			Uid uid = object.GetUid();
			ASSERT(_objects.find(uid) == _objects.end());
			_objects[uid] = & object;
			OnAddObject(object);
		}
	public:

		// coordinate system
		geom::abs::Vector3 const & GetOrigin() const
		{
			return _origin;
		}

		void SetOrigin(geom::abs::Vector3 const & origin)
		{
			OnSetOrigin(origin);

			_origin = origin;
		}

	private:
		virtual void OnSetOrigin(geom::abs::Vector3 const & origin)
		{
		}
	public:

#if defined(VERIFY)
		virtual void Verify() const
		{
			ForEachPair([] (std::pair<Uid, SmpObject *> const & pair) {
				SmpObject const & smp_object = ref(pair.second);
				ASSERT(pair.first == smp_object.GetUid());
				VerifyObject(smp_object);
			});

			VerifyObject(_origin);
		}
#endif

		////////////////////////////////////////////////////////////////////////////////
		// variables

	private:
		ObjectMap _objects;
		geom::abs::Vector3 _origin;
	};
}
