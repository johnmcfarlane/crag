//
//  Applet.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "Applet.h"

#include "ipc/Fiber.h"

#include "core/app.h"

using namespace applet;

////////////////////////////////////////////////////////////////////////////////
// applet::Applet member definitions

Applet::Applet(Engine & engine, char const * name, std::size_t stack_size, LaunchFunction const & function)
: super(engine)
, _fiber(name, stack_size, static_cast<void *>(this), & OnLaunch)
, _function(function)
, _wake_time(0)
, _quit_flag(false)
{
	ASSERT(_fiber.IsRunning());
}

Applet::~Applet()
{
	ASSERT(! _fiber.IsRunning());
	ASSERT(_quit_flag);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Applet, applet)
	CRAG_VERIFY(applet._fiber);
CRAG_VERIFY_INVARIANTS_DEFINE_END

bool Applet::IsRunning() const
{
	return _fiber.IsRunning();
}

core::Time Applet::GetWakeTime() const
{
	return _wake_time;
}

void Applet::Continue()
{
	_fiber.Continue();
}

void Applet::SetQuitFlag()
{
	CRAG_VERIFY(* this);
	_quit_flag = true;
}

bool Applet::GetQuitFlag() const
{
	CRAG_VERIFY(* this);
	return _quit_flag;
}

Engine & Applet::GetEngine() const
{
	return super::GetEngine();
}

char const * Applet::GetName() const
{
	return _fiber.GetName();
}

bool Applet::Sleep(core::Time duration)
{
	CRAG_VERIFY(* this);
	CRAG_VERIFY(! _quit_flag);

	_wake_time = duration + app::GetTime();
	_fiber.Yield();

	CRAG_VERIFY(* this);
	
	return ! _quit_flag;
}

void Applet::OnLaunch(void * data)
{
	Applet & applet = ref(reinterpret_cast<Applet *>(data));
	
	CRAG_VERIFY(applet);
	ASSERT(applet.IsRunning());
	
	applet._function(applet);
	CRAG_VERIFY(applet);

	applet.SetQuitFlag();

	ASSERT(applet.IsRunning());
}
