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

#if defined(__GNUC__)
#define SMP_LISTENER_ELLIPSIS
#else
#define SMP_LISTENER_ELLIPSIS ...
#endif

namespace smp
{
	// derive from this class to receive calls broadcast with the given parameters;
	// lives in the LISTENER_ENGINE thread and receives calls from SUBJECT_ENGINE
	template <typename SUBJECT_ENGINE, typename OBSERVER_ENGINE, typename SMP_LISTENER_ELLIPSIS PARAMETERS>
	class Listener : public ListenerInterface<SUBJECT_ENGINE, PARAMETERS SMP_LISTENER_ELLIPSIS>
	{
		enum State
		{
			normal,
			releasing,
			released
		};

		typedef ListenerInterface<SUBJECT_ENGINE, PARAMETERS SMP_LISTENER_ELLIPSIS> Subject;
		typedef OBSERVER_ENGINE ListenerEngine;
		typedef smp::Daemon<ListenerEngine> ListenerDaemon;

	public:
		// function
		Listener()
		: _state(normal)
		{
		}

		~Listener()
		{
			VerifyObject(* this);

			if (_state == normal)
			{
				BeginRelease();
			}

			while (! IsReleased())
			{
				Yield();
			}

			ASSERT(_state == released);
		}

		void BeginRelease()
		{
			VerifyObject(* this);

			ASSERT(_state == normal);
			_state = releasing;

			ListenerDaemon::Call([this] (ListenerEngine & engine) {
				Listener::_listeners.remove(* this);

				this->Acknowledge();
			});

			VerifyObject(* this);
		}

		bool IsReleased() const
		{
			VerifyObject(* this);

			ASSERT(_state == releasing || _state == released);
			return _state == released;
		}

#if defined(VERIFY)
		virtual void Verify() const final
		{
			switch (_state)
			{
				case normal:
				case releasing:
				case released:
					break;

				default:
					DEBUG_BREAK("bad enum value, %d", int(_state));
					break;
			}
		}
#endif

	private:
		virtual void Dispatch (PARAMETERS SMP_LISTENER_ELLIPSIS parameters) final
		{
			VerifyObject(* this);

			ListenerDaemon::Call([this, parameters SMP_LISTENER_ELLIPSIS] (ListenerEngine &) {
				(* this)(parameters SMP_LISTENER_ELLIPSIS);
			});

			VerifyObject(* this);
		}

		virtual void Acknowledge() final
		{
			VerifyObject(* this);
			ASSERT(_state == releasing);

			ListenerDaemon::Call([this] (ListenerEngine &) {
				ASSERT(_state == releasing);
				this->_state = released;
			});

			ASSERT(_state != released || _state != releasing);
			VerifyObject(* this);
		}

		virtual void operator() (PARAMETERS SMP_LISTENER_ELLIPSIS parameters) = 0;

		// variables
		State _state;
	};
}
