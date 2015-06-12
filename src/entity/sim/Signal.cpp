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

void Transmitter::SetReceiver(Receiver * r) noexcept
{
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
