//
//  AppletBase.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "AppletBase.h"

#include "Engine.h"
#include "TimeCondition.h"

#include "smp/Fiber.h"


using namespace script;


namespace
{
	// condition which never fails;
	// used by Yield to return ASAP;
	class NullCondition : public Condition
	{
		bool operator() ()
		{
			return true;
		}
	};

	NullCondition null_condition;
}


////////////////////////////////////////////////////////////////////////////////
// script::AppletBase member definitions

AppletBase::AppletBase()
: _fiber(ref(new smp::Fiber))
, _condition(& null_condition)
, _quit_flag(false)
{
	_fiber.Launch<AppletBase &>(* this);
}

AppletBase::~AppletBase()
{
	delete & _fiber;
}

bool AppletBase::IsRunning() const
{
	return _fiber.IsRunning();
}

Condition * AppletBase::GetCondition()
{
	return _condition;
}

void AppletBase::Continue()
{
	_fiber.Continue();
}

void AppletBase::operator() (smp::FiberInterface & fiber)
{
	ASSERT(& fiber == & _fiber);
	ASSERT(_condition == & null_condition);
	_condition = nullptr;

	(* this)(* this);

	ASSERT(_condition == nullptr);
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
	
	static_cast<smp::FiberInterface &>(_fiber).Yield();
	
	_condition = nullptr;
}

void AppletBase::Launch(AppletBase & script)
{
	script::Daemon::Call<AppletBase *>(& Engine::OnAddObject, & script);
}
