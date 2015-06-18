//
//  KeyboardTransmitter.cpp
//  crag
//
//  Created by John McFarlane on 2015-06-11.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "KeyboardTransmitter.h"

#include <sim/Engine.h>

#include <core/RosterObjectDefine.h>

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// KeyboardTransmitter member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	KeyboardTransmitter,
	10,
	Pool::Call<& KeyboardTransmitter::Tick>(Engine::GetTickRoster()))

KeyboardTransmitter::KeyboardTransmitter(SDL_Scancode key, SignalType down_signal, SignalType up_signal)
: _key(key)
, _down_signal(down_signal)
, _up_signal(up_signal)
{
	CRAG_VERIFY_OP(_down_signal, !=, _up_signal);
}

void KeyboardTransmitter::Tick()
{
	TransmitSignal(app::IsKeyDown(_key) ? _down_signal : _up_signal);
}
