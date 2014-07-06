//
//  Applet.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "Applet.h"

#include "AppletInterface_Impl.h"

#include "ipc/Fiber.h"

#include "core/app.h"

using namespace applet;

namespace
{
	// condition which never fails;
	// used by Yield to return ASAP;
	// only needed before apple's first Yield call
	bool null_condition()
	{
		return true;
	};
}


////////////////////////////////////////////////////////////////////////////////
// applet::Applet member definitions

Applet::Applet(Engine & engine, char const * name, std::size_t stack_size, LaunchFunction const & function)
: super(engine)
, _fiber(name, stack_size, static_cast<void *>(this), & OnLaunch)
, _function(function)
, _condition(null_condition)
, _quit_flag(false)
{
	ASSERT(_fiber.IsRunning());
}

Applet::~Applet()
{
	ASSERT(! _fiber.IsRunning());
	ASSERT(_quit_flag);
}

char const * Applet::GetName() const
{
	return _fiber.GetName();
}

bool Applet::IsRunning() const
{
	return _fiber.IsRunning();
}

const Condition & Applet::GetCondition() const
{
	return _condition;
}

void Applet::Continue()
{
	_fiber.Continue();
}

bool Applet::GetQuitFlag() const
{
	CRAG_VERIFY(* this);
	return _quit_flag;
}

void Applet::SetQuitFlag()
{
	CRAG_VERIFY(* this);
	_quit_flag = true;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Applet, applet)
	CRAG_VERIFY(applet._fiber);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Applet::Sleep(core::Time duration)
{
	auto wake_position = duration + app::GetTime();
	AppletInterface::WaitFor([this, wake_position] () {
		return (app::GetTime() >= wake_position) || _quit_flag;
	});
}

void Applet::WaitFor(Condition & condition)
{
	ASSERT(condition != null_condition);
	CRAG_VERIFY(* this);

	ASSERT(_condition == null_condition);
	_condition = condition;
	
	_fiber.Yield();
	
	_condition = null_condition;
}

Engine & Applet::GetEngine()
{
	return super::GetEngine();
}

void Applet::OnLaunch(void * data)
{
	Applet & applet = ref(reinterpret_cast<Applet *>(data));
	ASSERT(applet.IsRunning());
	
	ASSERT(applet._condition == null_condition);
	
	applet._function(applet);
	CRAG_VERIFY(applet);

	applet.SetQuitFlag();

	ASSERT(applet.IsRunning());
	ASSERT(applet._condition == null_condition);
}
