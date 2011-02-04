/*
 *  Singleton.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Singleton.h"
#include "Class.h"

#include "sim/Observer.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Simulation.h"


namespace
{
	
	////////////////////////////////////////////////////////////////////////////////
	// crag module Functions
	
	PyObject * Done(PyObject * self, PyObject * args)
	{
		vm::Singleton const & vm = vm::Singleton::Get();
		bool is_done = vm.IsDone();
		return Py_BuildValue("i", int(is_done));
	}
	
	PyObject * Time(PyObject * self, PyObject * args)
	{
		sim::SimulationPtr s(sim::Simulation::GetPtr());
		
		sim::Universe const & universe = s->GetUniverse();
		sys::TimeType time = universe.GetTime();
		return Py_BuildValue("d", time);
	}
	
	PyMethodDef crag_methods[] = 
	{
		{"Done", Done, METH_VARARGS, "Returns true iff it's time to quit."},
		{"Time", Time, METH_VARARGS, "Returns simulation time in seconds."},
		{NULL, NULL, 0, NULL}
	};
	
}


////////////////////////////////////////////////////////////////////////////////
// Singleton member definitions

vm::Singleton::Singleton(char const * init_source_filename)
: done(false)
, source_filename(init_source_filename)
{
	thread.Launch(* this);
}

vm::Singleton::~Singleton()
{
	// Communicate to the script that it's time to end.
	done = true;
	
	// Wait for the thread to be done.
	thread.Join();
}

bool vm::Singleton::IsDone() const
{
	return done;
}

void vm::Singleton::Run()
{
	Py_Initialize();
	
	{
		PyObject * crag_module = Py_InitModule("crag", crag_methods);
		Class<sim::Planet> planet_class("crag.Planet", "An Entity representing an astral body that has a surface.", crag_module);
		Class<sim::Observer> observer_class("crag.Observer", "An Entity representing the camera.", crag_module);
		Class<sim::Star> star_class("crag.Star", "An Entity representing an astral body that emits light.", crag_module);
		
		FILE * source_file = fopen(source_filename, "rt");
		PyRun_SimpleFileExFlags(source_file, source_filename, true, nullptr);
	}
	
	Py_Finalize();
	
	done = true;
}
