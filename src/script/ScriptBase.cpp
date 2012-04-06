//
//  ScriptBase.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "ScriptBase.h"

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
// script::ScriptBase member definitions

ScriptBase::ScriptBase()
: _fiber(ref(new smp::Fiber))
, _condition(& null_condition)
, _quit_flag(false)
{
	_fiber.Launch<ScriptBase &>(* this);
}

ScriptBase::~ScriptBase()
{
	delete & _fiber;
}

bool ScriptBase::IsRunning() const
{
	return _fiber.IsRunning();
}

Condition * ScriptBase::GetCondition()
{
	return _condition;
}

void ScriptBase::Continue()
{
	_fiber.Continue();
}

void ScriptBase::operator() (smp::FiberInterface & fiber)
{
	ASSERT(& fiber == & _fiber);
	ASSERT(_condition == & null_condition);
	_condition = nullptr;

	(* this)(* this);

	ASSERT(_condition == nullptr);
}

bool ScriptBase::GetQuitFlag() const
{
	return _quit_flag;
}

void ScriptBase::SetQuitFlag()
{
	_quit_flag = true;
}

// TODO: Should probably not be needed.
void ScriptBase::Yield()
{
	Wait(null_condition);
}

void ScriptBase::Sleep(Time duration)
{
	TimeCondition time_condition(duration);
	Wait(time_condition);
}

void ScriptBase::Wait(Condition & condition)
{
	ASSERT(_condition == nullptr);
	_condition = & condition;
	
	static_cast<smp::FiberInterface &>(_fiber).Yield();
	
	_condition = nullptr;
}

void ScriptBase::Launch(ScriptBase & script)
{
	script::Daemon::Call<ScriptBase *>(& Engine::OnAddObject, & script);
}
