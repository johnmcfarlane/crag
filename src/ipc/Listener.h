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

namespace ipc
{
	// derive from this class to receive calls broadcast with the given parameters;
	// lives in the LISTENER_ENGINE thread and receives calls from SUBJECT_ENGINE
	template <typename SUBJECT_ENGINE, typename OBSERVER_ENGINE, typename SMP_LISTENER_ELLIPSIS PARAMETERS>
	class Listener : public ListenerInterface<SUBJECT_ENGINE, PARAMETERS SMP_LISTENER_ELLIPSIS>
	{
		enum State
		{
			initializing,
			attached,
			releasing,
			released
		};

		typedef ListenerInterface<SUBJECT_ENGINE, PARAMETERS SMP_LISTENER_ELLIPSIS> Subject;
		typedef OBSERVER_ENGINE ListenerEngine;
		typedef ipc::Daemon<ListenerEngine> ListenerDaemon;

	public:
		// functions
		Listener()
		: _state(initializing)
		{
			VerifyObject(* this);

			Subject::SubjectDaemon::Call([this] (typename Subject::SubjectEngine & engine) {
				VerifyObject(* this);
				ASSERT(_state == initializing);

				this->Add();

				ListenerDaemon::Call([this] (ListenerEngine & engine) {
					VerifyObject(* this);
					ASSERT(this->_state == initializing);

					this->SetState(attached);
					VerifyObject(* this);
				});
			});
		}

		~Listener()
		{
			VerifyObject(* this);			
			ASSERT(IsListenerThread());
			ASSERT(IsReleased());
		}

		void BeginRelease()
		{
			VerifyObject(* this);
			ASSERT(IsListenerThread());

			ASSERT(_state == attached);
			SetState(releasing);

			Subject::SubjectDaemon::Call([this] (typename Subject::SubjectEngine & engine) {
				VerifyObject(* this);
				ASSERT(_state == releasing);

				// remove this from the list of listeners
				this->Remove();

				// send acknowledge back to listener's thread
				ListenerDaemon::Call([this] (ListenerEngine &) {
					VerifyObject(* this);
					ASSERT(this->_state == releasing);

					this->SetState(released);

					VerifyObject(* this);
				});

				ASSERT(_state != releasing || _state != released);
				VerifyObject(* this);
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
				case initializing:
					break;

				case attached:
					ASSERT(Subject::List::is_contained(* this));
					break;

				case releasing:
					break;

				case released:
					ASSERT(! Subject::List::is_contained(* this));
					break;

				default:
					DEBUG_BREAK("bad enum value, %d", int(_state));
					break;
			}
		}
#endif

	private:
		static bool IsListenerThread()
		{
			return ListenerDaemon::IsCurrentThread();
		}

		void SetState(State state)
		{
			ASSERT(IsListenerThread());
			ASSERT(state > initializing && state <= released);
			ASSERT(int(state) == int(_state) + 1);
			_state = state;
		}

		virtual void Dispatch (PARAMETERS SMP_LISTENER_ELLIPSIS parameters) final
		{
			VerifyObject(* this);
			ASSERT(Subject::SubjectDaemon::IsCurrentThread());

			ListenerDaemon::Call([this, parameters SMP_LISTENER_ELLIPSIS] (ListenerEngine &) {
				(* this)(parameters SMP_LISTENER_ELLIPSIS);
			});

			VerifyObject(* this);
		}

		virtual void operator() (PARAMETERS SMP_LISTENER_ELLIPSIS parameters) = 0;

		// variables
		State _state;
	};
}
