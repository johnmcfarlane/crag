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
		typedef ListenerInterface<SUBJECT_ENGINE, PARAMETERS SMP_LISTENER_ELLIPSIS> Subject;
		typedef OBSERVER_ENGINE ListenerEngine;
		typedef ipc::Daemon<ListenerEngine> ListenerDaemon;

	public:
		// functions
		Listener()
		: _is_initialized(false)
		, _is_released(false)
		{
			VerifyObject(* this);

			++ Subject::_counter;

			Subject::SubjectDaemon::Call([this] (typename Subject::SubjectEngine &) {
				VerifyObject(* this);
				ASSERT(! _is_initialized);
				ASSERT(! _is_released);

				this->Add();

				ListenerDaemon::Call([this] (ListenerEngine &) {
					VerifyObject(* this);
					ASSERT(! this->_is_initialized);
					ASSERT(! this->_is_released);

					this->_is_initialized = true;

					VerifyObject(* this);
				});
			});
		}

		~Listener()
		{
			VerifyObject(* this);
#if ! defined(__ANDROID__)
			ASSERT(IsListenerThread());
#endif
			ASSERT(_is_released);
		}

		void BeginRelease()
		{
#if ! defined(__ANDROID__)
			ASSERT(IsListenerThread());
#endif

			VerifyObject(* this);
			ASSERT(! _is_released);

			Subject::SubjectDaemon::Call([this] (typename Subject::SubjectEngine &) {
				VerifyObject(* this);
				ASSERT(! _is_released);

				// remove this from the list of listeners
				this->Remove();

				// send acknowledge back to listener's thread
				ListenerDaemon::Call([this] (ListenerEngine &) {
					VerifyObject(* this);
					ASSERT(_is_initialized);
					ASSERT(! _is_released);
					
					this->_is_released = true;
					-- Subject::_counter;

					VerifyObject(* this);
				});

				VerifyObject(* this);
			});

			VerifyObject(* this);
		}

		bool IsReleased() const
		{
			VerifyObject(* this);

			return _is_released;
		}
		
#if defined(VERIFY)
		virtual void Verify() const final
		{
			VerifyOp(Subject::_counter, >=, 0);
			
			if (_is_initialized)
			{
				if (_is_released)
				{
					VerifyTrue(! Subject::List::is_contained(* this));
				}
			}
		}
#endif

	private:
		static bool IsListenerThread()
		{
			return ListenerDaemon::IsCurrentThread();
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
		bool _is_initialized;
		bool _is_released;
	};
}
