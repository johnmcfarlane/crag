//
//  AppletBase.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "AppletBase.h"

#include "TimeCondition.h"

#include "smp/Fiber.h"


using namespace applet;


namespace
{
	// condition which never fails;
	// used by Yield to return ASAP;
	class NullCondition : public Condition
	{
		bool operator() (bool hurry)
		{
			return true;
		}
	};

	NullCondition null_condition;
}


////////////////////////////////////////////////////////////////////////////////
// applet::AppletBase member definitions

AppletBase::AppletBase(super::Init const & init)
: super(init)
, _fiber(ref(new smp::Fiber))
, _condition(& null_condition)
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

Condition * AppletBase::GetCondition()
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
	Wait(null_condition);
}

void AppletBase::Sleep(Time duration)
{
	TimeCondition time_condition(duration);
	Wait(time_condition);
}

void AppletBase::Wait(Condition & condition)
{
	ASSERT(_condition == nullptr);
	_condition = & condition;
	
	_fiber.Yield();
	
	_condition = nullptr;
}

void AppletBase::OnLaunch(void * data)
{
	AppletBase & applet = ref(reinterpret_cast<AppletBase *>(data));
	ASSERT(applet._finished_flag == false);
	
	ASSERT(applet._condition == & null_condition);
	applet._condition = nullptr;
	
	applet(applet);
	
	ASSERT(applet._finished_flag == false);
	applet._finished_flag = true;
	
	ASSERT(applet._condition == nullptr);
}
