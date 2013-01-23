//
//  AppletBase.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "AppletBase.h"

#include "AppletInterface_Impl.h"

#include "smp/Fiber.h"

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
// applet::AppletBase member definitions

AppletBase::AppletBase(super::Init const & init, char const * name, std::size_t stack_size)
: super(init)
, _fiber(ref(new smp::Fiber(name, stack_size, static_cast<void *>(this), & OnLaunch)))
, _condition(null_condition)
, _quit_flag(false)
{
	ASSERT(_fiber.IsRunning());
}

AppletBase::~AppletBase()
{
	ASSERT(! _fiber.IsRunning());
	ASSERT(_quit_flag);
	
	delete & _fiber;
}

char const * AppletBase::GetName() const
{
	return _fiber.GetName();
}

bool AppletBase::IsRunning() const
{
	return _fiber.IsRunning();
}

const Condition & AppletBase::GetCondition() const
{
	return _condition;
}

void AppletBase::Continue()
{
	_fiber.Continue();
}

bool AppletBase::GetQuitFlag() const
{
	VerifyObject(* this);
	return _quit_flag;
}

void AppletBase::SetQuitFlag()
{
	VerifyObject(* this);
	_quit_flag = true;
}

#if defined(VERIFY)
void AppletBase::Verify() const
{
	VerifyObject(_fiber);
}
#endif

void AppletBase::Sleep(core::Time duration)
{
	auto wake_position = duration + app::GetTime();
	AppletInterface::WaitFor([this, wake_position] () {
		return (app::GetTime() >= wake_position) || _quit_flag;
	});
}

void AppletBase::WaitFor(Condition & condition)
{
	ASSERT(condition != null_condition);
	VerifyObject(* this);

	ASSERT(_condition == null_condition);
	_condition = condition;
	
	_fiber.Yield();
	
	_condition = null_condition;
}

Engine & AppletBase::GetEngine()
{
	return super::GetEngine();
}

void AppletBase::OnLaunch(void * data)
{
	AppletBase & applet = ref(reinterpret_cast<AppletBase *>(data));
	ASSERT(applet.IsRunning());
	
	ASSERT(applet._condition == null_condition);
	
	applet(applet);
	VerifyObject(applet);

	ASSERT(applet.IsRunning());
	ASSERT(applet._condition == null_condition);
}
