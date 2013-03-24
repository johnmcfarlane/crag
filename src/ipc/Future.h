//
//  Future.h
//  crag
//
//  Created by John McFarlane on 2012-04-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace ipc
{
	// A deferred result from a cross-thread function call.
	// This is a good way to call a function which returns a value
	// without having to wait for the result straight away.
	template <typename VALUE>
	class Future
	{
	public:
		enum Status
		{
			pending,	// result has not yet been written
			success,	// result has been written
			failure		// target object did not exist
		};

		Future()
			: _status(pending)
		{
		}

		~Future()
		{
			ASSERT(IsComplete());
			verify();
		}
		
		bool IsPending() const
		{
			verify();
			return _status == pending;
		}
		
		bool IsComplete() const
		{
			verify();
			return _status == success || _status == failure;
		}
		
		bool IsValid() const
		{
			verify();
			return _status == success;
		}
		
		// blocks until valid and then returns result of function call given in c'tor
		VALUE & Get()
		{
			ASSERT(_status == success);
			return _value;
		}
		
		// receiver responds with either of these two functions
		void OnSuccess(VALUE const & value)
		{
			ASSERT(IsPending());
			
			_value = value;
			std::atomic_thread_fence(std::memory_order_seq_cst);
			_status = success;
		}
		
		void OnFailure()
		{
			ASSERT(IsPending());
			
			_status = failure;
		}
		
	private:
		void verify() const
		{
			ASSERT((_status == pending) || (_status == success) || (_status == failure));
		}
		
		// variables
		VALUE _value;
		Status _status;
	};
}
