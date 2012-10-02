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
	bool null_condition_function(bool quit_flag)
	{
		return true;
	};
	
	Condition null_condition(null_condition_function);
}


////////////////////////////////////////////////////////////////////////////////
// applet::AppletBase member definitions

AppletBase::AppletBase(super::Init const & init)
: super(init)
, _fiber(ref(new smp::Fiber))
, _condition(null_condition)
, _quit_flag(false)
, _finished_flag(false)
{
	_fiber.Launch(& OnLaunch, this);
}

AppletBase::~AppletBase()
{
	delete & _fiber;
}

bool AppletBase::IsRunning() const
{
	return ! _finished_flag;
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
	return _quit_flag;
}

void AppletBase::SetQuitFlag()
{
	_quit_flag = true;
}

// TODO: Should probably not be needed.
void AppletBase::Yield()
{
	WaitFor(null_condition);
}

void AppletBase::Sleep(core::Time duration)
{
	auto wake_position = duration + app::GetTime();
	AppletInterface::WaitFor([wake_position] (bool quit_flag) {
		return (app::GetTime() >= wake_position) | quit_flag;
	});
}

void AppletBase::WaitFor(Condition & condition)
{
	ASSERT(_condition == null_condition);
	_condition = condition;
	
	_fiber.Yield();
	
	_condition = null_condition;
}

void AppletBase::OnLaunch(void * data)
{
	AppletBase & applet = ref(reinterpret_cast<AppletBase *>(data));
	ASSERT(applet._finished_flag == false);
	
	ASSERT(applet._condition == null_condition);
	
	applet(applet);
	
	ASSERT(applet._finished_flag == false);
	applet._finished_flag = true;
	
	ASSERT(applet._condition == null_condition);
}
