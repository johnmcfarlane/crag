//
//  ListenerInterface.h
//  crag
//
//  Created by John on 2013-02-19.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/SimpleMutex.h"

namespace ipc
{
	// hack to get around not being able to put nodes in virtual base classes
	class ListenerBase
	{
	public:
		// true if it's currently safe to shut down from point of view of the Listener system
		static bool CanExit();
		
	protected:
		typedef smp::SimpleMutex Mutex;
		DEFINE_INTRUSIVE_LIST(ListenerBase, List);
		
		static int _counter;
		static Mutex _mutex;
	};
	
	// see ipc::Listener and ipc::Daemon::Broadcast for usage;
	// provides thread-agnostic interface to Listener class;
	// keeps a list of instances for broadcasting to
	template <typename EVENT>
	class ListenerInterface : public ListenerBase
	{
	public:
		// functions
		virtual ~ListenerInterface()
		{
#if defined(CRAG_DEBUG)
			std::lock_guard<Mutex> lock(_mutex);

			if (_listeners.contains(* this))
			{
				DEBUG_BREAK("Listener was still around!");
			}
#endif
		}

		// call all Listener objects with the given event
		static void Broadcast (EVENT event)
		{
			std::lock_guard<Mutex> lock(_mutex);
			// With enough listeners, this lock will become prohibitively expensive.
			// When that happens, _listeners can be swapped into an empty list before
			// being looped over and merged back in after.

			for (auto & listener_node : _listeners)
			{
				auto & listener = static_cast<ListenerInterface &>(listener_node);
				CRAG_VERIFY_TRUE(& listener);
				CRAG_VERIFY(& listener);

				listener.Dispatch(event);
			}
		}

	protected:
		void Add()
		{
			++ _counter;
			ASSERT(_counter > 0);

			std::lock_guard<Mutex> lock(_mutex);

			ASSERT(! _listeners.contains(* this));
			_listeners.push_back(* this);
			ASSERT(_listeners.contains(* this));
		}

		void Remove()
		{
			std::lock_guard<Mutex> lock(_mutex);

			ASSERT(_listeners.contains(* this));
			_listeners.remove(* this);
			ASSERT(! _listeners.contains(* this));

			ASSERT(_counter > 0);
			-- _counter;
		}
		
		bool IsContained() const
		{
			std::lock_guard<Mutex> lock(_mutex);
			bool is_contained = List::is_contained(* this);
			ASSERT(is_contained == _listeners.contains(* this));
			return is_contained;
		}

	private:
		virtual void Dispatch(EVENT event) = 0;

		// variables
		static List _listeners;
	};

	template <typename EVENT>
	typename ListenerInterface<EVENT>::List ListenerInterface<EVENT>::_listeners;
}
