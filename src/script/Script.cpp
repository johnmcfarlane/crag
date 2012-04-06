//
//  Script.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "Script.h"

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
// script::Script member definitions

Script::Script()
: _fiber(ref(new smp::Fiber))
, _condition(& null_condition)
, _quit_flag(false)
{
	_fiber.Launch<Script &>(* this);
}

Script::~Script()
{
	delete & _fiber;
}

bool Script::IsRunning() const
{
	return _fiber.IsRunning();
}

Condition * Script::GetCondition()
{
	return _condition;
}

void Script::Continue()
{
	_fiber.Continue();
}

void Script::operator() (smp::FiberInterface & fiber)
{
	ASSERT(& fiber == & _fiber);
	ASSERT(_condition == & null_condition);
	_condition = nullptr;

	(* this)(* this);

	ASSERT(_condition == nullptr);
}

bool Script::GetQuitFlag() const
{
	return _quit_flag;
}

void Script::SetQuitFlag()
{
	_quit_flag = true;
}

// TODO: Should probably not be needed.
void Script::Yield()
{
	Wait(null_condition);
}

void Script::Sleep(Time duration)
{
	TimeCondition time_condition(duration);
	Wait(time_condition);
}

void Script::Wait(Condition & condition)
{
	ASSERT(_condition == nullptr);
	_condition = & condition;
	
	static_cast<smp::FiberInterface &>(_fiber).Yield();
	
	_condition = nullptr;
}

void Script::Launch(Script & script)
{
	script::Daemon::Call<Script *>(& Engine::OnAddObject, & script);
}
