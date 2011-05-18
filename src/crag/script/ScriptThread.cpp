/*
 *  ScriptThread.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "ScriptThread.h"
#include "Class.h"

#include "sim/Observer.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Simulation.h"
#include "sim/Universe.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


FILE_LOCAL_BEGIN

////////////////////////////////////////////////////////////////////////////////
// crag module Functions

PyObject * time(PyObject * self, PyObject * args)
{
	sim::Simulation & simulation = sim::Simulation::Ref();

	sys::TimeType time = simulation.GetTime();
	return Py_BuildValue("d", time);
}

PyObject * get_event(PyObject * self, PyObject * args)
{
	sys::Event event;
	bool blocking = true;
	while (sys::GetEvent(event, blocking))
	{
		if (sim::Simulation::Ref().HandleEvent(event))
		{
			// The simulation object caught the event.
			continue;
		}
		
		// Return details of certain events back to the script.
		switch (event.type)
		{
			case SDL_QUIT:
				return Py_BuildValue("si", "exit", 0);
				
			case SDL_KEYDOWN:
				return Py_BuildValue("sii", "keydown", event.key.keysym.scancode, 1);
				
			case SDL_KEYUP:
				return Py_BuildValue("sii", "keydown", event.key.keysym.scancode, 0);
				
			case SDL_MOUSEBUTTONDOWN:
				return Py_BuildValue("si", "buttondown", event.button.button);
				
			case SDL_MOUSEMOTION:
				return Py_BuildValue("sii", "mousemove", event.motion.xrel, event.motion.yrel);
		}
	}
	
	// There was an error. That's all the docs say.
	Assert(false);
	return nullptr;
}

PyMethodDef crag_methods[] = 
{
	{"time", time, METH_VARARGS, "Returns simulation time in seconds."},
	{"get_event", get_event, METH_VARARGS, "Returns the next event in the event queue."},
	{NULL, NULL, 0, NULL}
};


////////////////////////////////////////////////////////////////////////////////
// ScriptThread member definitions

PyObject * observer_add_rotation(PyObject * self, PyObject * args)
{
	sim::Vector3 rotations;
	if (! PyArg_ParseTuple(args, "ddd", & rotations.x, & rotations.y, & rotations.z))
	{
		return nullptr;
	}
	
	sim::Observer * observer = script::Class<sim::Observer>::GetNativeObject(self);
	observer->AddRotation(rotations);

	Py_RETURN_NONE;
}

PyMethodDef observer_methods[] = {
	{"add_rotation", (PyCFunction)observer_add_rotation, METH_VARARGS, "Add some rotational impulse to the observer"},
	//{"add_thrust", (PyCFunction)observer_add_thrust, METH_VARARGS, "Add some thrust impulse to the observer"},
	{NULL}  /* Sentinel */
};

FILE_LOCAL_END


////////////////////////////////////////////////////////////////////////////////
// ScriptThread member definitions

script::ScriptThread::ScriptThread()
{
}

script::ScriptThread::~ScriptThread()
{
}

// Note: Run should be called from same thread as c'tor/d'tor.
void script::ScriptThread::Run()
{
	Py_Initialize();
	Run("./script/main.py");
	exit(0);	// TODO
	Py_Finalize();
}

void script::ScriptThread::Run(char const * source_filename)
{
	PyObject * crag_module = Py_InitModule("crag", crag_methods);
	Class<sim::Entity> entity_class("crag.Entity", * crag_module, "An Entity.", nullptr);
	Class<sim::Planet> planet_class("crag.Planet", * crag_module, "An Entity representing an astral body that has a surface.", nullptr, entity_class);
	Class<sim::Observer> observer_class("crag.Observer", * crag_module, "An Entity representing the camera.", observer_methods, entity_class);
	Class<sim::Star> star_class("crag.Star", * crag_module, "An Entity representing an astral body that emits light.", nullptr, entity_class);	
	
	char mode[] = "r";
	PyObject* PyFileObject = PyFile_FromString(const_cast<char *>(source_filename), mode);
	PyRun_SimpleFile(PyFile_AsFile(PyFileObject), source_filename);
}