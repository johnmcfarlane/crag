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

Transmitter::Transmitter(Receiver * r) noexcept
: receiver(r)
{
}

Transmitter::~Transmitter() noexcept
{
}

void Transmitter::SetReceiver(Receiver * r) noexcept
{
	// no reason not to overwrite a pointer but currently not done
	CRAG_VERIFY_TRUE(r);
	CRAG_VERIFY_FALSE(receiver);

	receiver = r;
}

Receiver * Transmitter::GetReceiver() const noexcept
{
	return receiver;
}

void Transmitter::TransmitSignal(SignalType v) noexcept
{
	receiver->ReceiveSignal(v);
}
