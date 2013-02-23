//
//  Listener.h
//  crag
//
//  Created by John on 2013-02-19.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace smp
{
	// hack to get around not being able to put nodes in virtual base classes
	class ListenerNode
	{
		DEFINE_INTRUSIVE_LIST(ListenerNode, List);
	};

	// see smp::Listener and smp::Daemon::Broadcast for usage;
	// provides thread-agnostic interface to Listener class;
	// keeps a list of instances for broadcasting to
	template <typename ENGINE, typename ... PARAMETERS>
	class ListenerInterface : public ListenerNode
	{
	public:
		// types
		typedef ENGINE SubjectEngine;
		typedef smp::Daemon<SubjectEngine> SubjectDaemon;

		// functions
		virtual ~ListenerInterface()
		{
#if defined(NDEBUG)
			SubjectDaemon::Call([this] (SubjectEngine & engine) {
				if (_listeners.contains(this))
				{
					DEBUG_BREAK("Listener was still around!");
				}
			});
#endif
		}

		// call all Listener objects with the given parameters
		static void Broadcast (PARAMETERS ... parameters)
		{
			ASSERT(IsSubjectThread());
			for (auto & listener_node : _listeners)
			{
				auto & listener = static_cast<ListenerInterface &>(listener_node);
				VerifyRef(listener);

				listener.Dispatch(parameters ...);
			}
		}

#if defined(VERIFY)
		virtual void Verify() const = 0;
#endif

	protected:
		static bool IsSubjectThread()
		{
			return SubjectDaemon::IsCurrentThread();
		}

		void Add()
		{
			ASSERT(IsSubjectThread());
			_listeners.push_back(* this);
		}

		void Remove()
		{
			ASSERT(IsSubjectThread());
			_listeners.remove(* this);
		}

	private:
		virtual void Dispatch(PARAMETERS ... parameters) = 0;

		// variables
		static List _listeners;
	};

	template <typename ENGINE, typename ... PARAMETERS>
	typename ListenerInterface<ENGINE, PARAMETERS ...>::List ListenerInterface<ENGINE, PARAMETERS ...>::_listeners;
}
