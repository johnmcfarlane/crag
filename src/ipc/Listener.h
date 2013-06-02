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
			VerifyObject(* this);
			
			return _is_listening || _dispatch_count > 0;
		}
		
		bool IsListening() const
		{
			VerifyObject(* this);
			
			return _is_listening;
		}
		
		void SetIsListening(bool is_listening)
		{
			VerifyObject(* this);
			
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
			
			VerifyObject(* this);
		}

#if defined(VERIFY)
		void Verify() const
		{
			//VerifyEqual(_super::IsContained(), _is_listening);
			VerifyOp(_super::_counter, >=, 0);
		}
#endif

	private:
		static bool IsListenerThread()
		{
			return ListenerDaemon::IsCurrentThread();
		}
		
		virtual void Dispatch (EVENT event) final
		{
			VerifyObject(* this);
			_dispatch_count += 1;

			ListenerDaemon::Call([this, event] (ListenerEngine &) {
				(* this)(event);
				_dispatch_count -= 1;
			});

			VerifyObject(* this);
		}

		virtual void operator() (EVENT const & event) = 0;
		
		// variables
		bool _is_listening;
		std::atomic<unsigned> _dispatch_count;
	};
}
