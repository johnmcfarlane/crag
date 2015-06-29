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

	// transmitting end of a signal; must point to one or more Receivers
	class Transmitter
		: public crag::counted_object<Receiver>
	{
	public:
		// functions
		Transmitter() noexcept;
		virtual ~Transmitter() noexcept;

		void AddReceiver(Receiver & r) noexcept;

		void TransmitSignal(SignalType v) noexcept;

		template <typename FUNCTION>
		void ForEachReceiver(FUNCTION function) const noexcept
		{
			for (auto receiver : receivers)
			{
				if (receiver)
				{
					function(* receiver);
				}
			}
		}

	private:
		// constants
		static constexpr auto max_num_receivers = 3;

		// types
		using ReceiverArray = std::array<Receiver *, max_num_receivers>;

		// variables
		ReceiverArray receivers {{ nullptr, nullptr, nullptr }};
	};
}
