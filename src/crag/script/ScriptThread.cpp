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
#include "MetaClass.h"

#include "sim/Simulation.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif

// Compiling python yourself? My preference:
// CFLAGS="-arch i386" ./configure --with-universal-archs=intel --enable-universalsdk=/Developer/SDKs/MacOSX10.6.sdk --prefix=$HOME --enable-stacklessfewerregisters

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
				return Py_BuildValue("sii", "key", event.key.keysym.scancode, 1);
				
			case SDL_KEYUP:
				return Py_BuildValue("sii", "key", event.key.keysym.scancode, 0);
				
			case SDL_MOUSEBUTTONDOWN:
				return Py_BuildValue("si", "mousebutton", event.button.button, 1);
				
			case SDL_MOUSEBUTTONUP:
				return Py_BuildValue("si", "mousebutton", event.button.button, 0);
				
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

static PyModuleDef crag_module_def = {
    PyModuleDef_HEAD_INIT, "crag", NULL, -1, crag_methods,
    NULL, NULL, NULL, NULL
};

PyObject * create_crag_module()
{
    // Create the module
    PyObject & crag_module = ref(PyModule_Create(& crag_module_def));

	// Register the classes
	for (script::MetaClassPoly::iterator i = script::MetaClassPoly::begin(); i != script::MetaClassPoly::end(); ++ i)
	{
		script::MetaClassPoly & meta_class = * i;
		meta_class.Init(crag_module, "crag", "No documentation available. TBD");
	}
    
    return & crag_module;
}


FILE_LOCAL_END


////////////////////////////////////////////////////////////////////////////////
// ScriptThread member definitions

script::ScriptThread::ScriptThread()
{
}

// Note: Run should be called from same thread as c'tor/d'tor.
void script::ScriptThread::Run()
{
    if (PyImport_AppendInittab("crag", & create_crag_module))
	{
		std::cout << "failed to create crag python module.\n";
		return;
	}
    
	Py_Initialize();
	Run("./script/main.py");
	Py_Finalize();
}

void script::ScriptThread::Run(char const * source_filename)
{
	FILE * file = fopen(source_filename, "r");
	PyRun_SimpleFileEx(file, source_filename, true);
}

// TODO: Add documentation back in:
//Class<sim::Entity> entity_class("crag.Entity", * crag_module, "An Entity.", nullptr);
//Class<sim::Planet> planet_class("crag.Planet", * crag_module, "An Entity representing an astral body that has a surface.", nullptr, entity_class);
//Class<sim::Observer> observer_class("crag.Observer", * crag_module, "An Entity representing the camera.", observer_methods, entity_class);
//Class<sim::Star> star_class("crag.Star", * crag_module, "An Entity representing an astral body that emits light.", nullptr, entity_class);
