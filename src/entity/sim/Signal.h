//
//  entity/sim/Signal.h
//  crag
//
//  Created by John McFarlane on 2015-06-09.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <core/counted_object.h>

namespace sim
{
	using SignalType = float;
	class Transmitter;

	// receiving end of a signal; must be pointer to by a Transmitter
	class Receiver
		: public crag::counted_object<Receiver>
	{
		friend class Transmitter;
	public:
		// functions
		Receiver(SignalType v = 0);
		virtual ~Receiver() noexcept;

		SignalType GetSignal() const noexcept;
	private:
		void ReceiveSignal(SignalType v) noexcept;

		// variables
		SignalType value = 0;
	};

	// transmitting end of a signal; must point to a Transmitter
	class Transmitter
		: public crag::counted_object<Receiver>
	{
	public:
		// functions
		Transmitter(Receiver * r = nullptr) noexcept;
		virtual ~Transmitter() noexcept;

		void SetReceiver(Receiver * r) noexcept;
		Receiver * GetReceiver() const noexcept;

		void TransmitSignal(SignalType v) noexcept;

	private:
		// variables
		Receiver * receiver = nullptr;
	};
}
