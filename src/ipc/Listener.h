//
//  Listener.h
//  crag
//
//  Created by John on 2013-02-19.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ListenerInterface.h"

namespace ipc
{
	// derive from this class to receive calls broadcast with the given event;
	// lives in the LISTENER_ENGINE thread
	template <typename ENGINE, typename EVENT>
	class Listener : public ListenerInterface<EVENT>
	{
		typedef ListenerInterface<EVENT> _super;
		typedef ENGINE ListenerEngine;
		typedef ipc::Daemon<ListenerEngine> ListenerDaemon;

	public:
		// functions
		Listener()
		: _is_listening(false)
		, _dispatch_count(0)
		{
			SetIsListening(true);
		}

		~Listener()
		{
			ASSERT(! IsBusy());
			
			SetIsListening(false);
		}
		
		// busy objects should not be destroyed
		bool IsBusy() const
		{
			CRAG_VERIFY(* this);
			
			return _is_listening || _dispatch_count > 0;
		}
		
		bool IsListening() const
		{
			CRAG_VERIFY(* this);
			
			return _is_listening;
		}
		
		void SetIsListening(bool is_listening)
		{
			CRAG_VERIFY(* this);
			
			if (_is_listening == is_listening)
			{
				return;
			}
			_is_listening = is_listening;
			
			if (is_listening)
			{
				_super::Add();
			}
			else			
			{
				_super::Remove();
			}
			
			CRAG_VERIFY(* this);
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Listener, self)
			//CRAG_VERIFY_EQUAL(_super::IsContained(), _is_listening);
			CRAG_VERIFY_OP(self._counter, >=, 0);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

	private:
		static bool IsListenerThread()
		{
			return ListenerDaemon::IsCurrentThread();
		}
		
		virtual void Dispatch (EVENT event) final
		{
			CRAG_VERIFY(* this);
			_dispatch_count += 1;

			ListenerDaemon::Call([this, event] (ListenerEngine &) {
				(* this)(event);
				_dispatch_count -= 1;
			});

			CRAG_VERIFY(* this);
		}

		virtual void operator() (EVENT const & event) = 0;
		
		// variables
		bool _is_listening;
		std::atomic<unsigned> _dispatch_count;
	};
}
