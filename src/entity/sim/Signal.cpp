//
//  entity/sim/Signal.cpp
//  crag
//
//  Created by John McFarlane on 2015-06-11.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Signal.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// Receiver member defintions

Receiver::Receiver(SignalType v)
: value(v)
{
}

Receiver::~Receiver() noexcept
{
}

SignalType Receiver::GetSignal() const noexcept
{
	return value;
}

void Receiver::ReceiveSignal(SignalType v) noexcept
{
	value = v;
}

////////////////////////////////////////////////////////////////////////////////
//  member defintions

Transmitter::Transmitter() noexcept
{
	static_assert(sizeof(Transmitter) == 4 * sizeof(Receiver *), "type isn't a neat size");
	CRAG_VERIFY_TRUE(std::all_of(std::begin(receivers), std::end(receivers), [] (Receiver * r) { return r == nullptr; }));
}

Transmitter::~Transmitter() noexcept
{
}

void Transmitter::AddReceiver(Receiver & r) noexcept
{
	for (auto & receiver : receivers)
	{
		if (! receiver)
		{
			receiver = & r;
			return;
		}
	}

	DEBUG_BREAK("Transmitter is full");
}

void Transmitter::TransmitSignal(SignalType v) noexcept
{
	ForEachReceiver([&] (Receiver & r)
	{
		r.ReceiveSignal(v);
	});
}
