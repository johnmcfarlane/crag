//
//  MessageQueue.h
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/SimpleMutex.h"
#include "smp/smp.h"

#include "core/ring_buffer.h"


namespace ipc
{
	template <typename CLASS>
	class MessageQueue
	{
		//////////////////////////////////////////////////////////////////////////////
		// types

		class EnvelopeBase;

		template <typename MESSAGE>
		class Envelope;
	
		typedef CLASS Class;
		typedef core::ring_buffer<EnvelopeBase, true> Buffer;

		typedef smp::SimpleMutex Mutex;
		typedef std::lock_guard<Mutex> Lock;
	public:
		typedef typename Buffer::size_type size_type;
		
		//////////////////////////////////////////////////////////////////////////////
		// functions

		MessageQueue(size_type capacity);

		bool IsEmpty() const;
		
		void Clear();
		
		// adds message to queue
		template <typename MESSAGE>
		void PushBack(MESSAGE const & object);
		
		// returns false iff the Daemon should quit
		bool DispatchMessage(Class & object);
		
		// returns number of messages process
		int DispatchMessages(Class & object);
		
	private:
		template <typename MESSAGE>
		void CompleteDispatch(MESSAGE message, Class & object);
		
		//////////////////////////////////////////////////////////////////////////////
		// variables

		Mutex _mutex;
		Buffer _buffer;
	};
}
