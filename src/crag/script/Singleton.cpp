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
		script::Singleton const & script = script::Singleton::Get();
		bool is_done = script.IsDone();
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
		{"done", Done, METH_VARARGS, "Returns true iff it's time to quit."},
		{"time", Time, METH_VARARGS, "Returns simulation time in seconds."},
		{NULL, NULL, 0, NULL}
	};

}


////////////////////////////////////////////////////////////////////////////////
// Singleton member definitions

script::Singleton::Singleton(char const * init_source_filename)
: done(false)
, source_filename(init_source_filename)
{
	thread.Launch(* this);
}

script::Singleton::~Singleton()
{
	// Communicate to the script that it's time to end.
	done = true;
	
	// Wait for the thread to be done.
	thread.Join();
}

bool script::Singleton::IsDone() const
{
	return done;
}

void script::Singleton::Run()
{
	Py_Initialize();
	
	{
		PyObject * crag_module = Py_InitModule("crag", crag_methods);
		Class<sim::Entity> entity_class("crag.Entity", * crag_module, "An Entity.");
		Class<sim::Planet> planet_class("crag.Planet", * crag_module, "An Entity representing an astral body that has a surface.", entity_class);
		Class<sim::Observer> observer_class("crag.Observer", * crag_module, "An Entity representing the camera.", entity_class);
		Class<sim::Star> star_class("crag.Star", * crag_module, "An Entity representing an astral body that emits light.", entity_class);

		char mode[] = "r";
		PyObject* PyFileObject = PyFile_FromString(const_cast<char *>(source_filename), mode);
		PyRun_SimpleFile(PyFile_AsFile(PyFileObject), source_filename);
	}
	
	Py_Finalize();
	
	done = true;
}
