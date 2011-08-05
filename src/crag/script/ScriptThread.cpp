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

// Compiling stackless python on OS X yourself? My preference:
// CFLAGS="-arch i386" ./configure --with-universal-archs=intel --enable-universalsdk=/Developer/SDKs/MacOSX10.6.sdk --prefix=$HOME --enable-stacklessfewerregisters

FILE_LOCAL_BEGIN

////////////////////////////////////////////////////////////////////////////////
// crag module Functions

PyObject * time(PyObject * /*self*/, PyObject * /*args*/)
{
	sim::Simulation & simulation = sim::Simulation::Ref();

	sys::TimeType time = simulation.GetTime();
	return Py_BuildValue("d", time);
}

PyObject * sleep(PyObject * /*self*/, PyObject * args)
{
	// Get the time parameter.
	sys::TimeType delay;
	if (! PyArg_ParseTuple(args, "d", & delay))
	{
		delay = 0;
	}
	
	smp::Sleep(delay);
	
	Py_RETURN_NONE;
}

PyObject * get_event(PyObject * /*self*/, PyObject * /*args*/)
{
	script::ScriptThread & script_thread = script::ScriptThread::Ref();
	return script_thread.PollEvent();
}

PyObject * create_event_object(sys::Event const & event)
{
	switch (event.type)
	{
		case SDL_QUIT:
			return Py_BuildValue("si", "exit", 0);
			
		case SDL_KEYDOWN:
			return Py_BuildValue("sii", "key", event.key.keysym.sym, 1);
			
		case SDL_KEYUP:
			return Py_BuildValue("sii", "key", event.key.keysym.sym, 0);
			
		case SDL_MOUSEBUTTONDOWN:
			return Py_BuildValue("si", "mousebutton", event.button.button, 1);
			
		case SDL_MOUSEBUTTONUP:
			return Py_BuildValue("si", "mousebutton", event.button.button, 0);
			
		case SDL_MOUSEMOTION:
			return Py_BuildValue("sii", "mousemove", event.motion.xrel, event.motion.yrel);
			
		default:
			return nullptr;
	}
}

bool handle_events(PyObject * & event_object)
{
	// If no events are pending,
	sys::Event event;
	if (! sys::GetEvent(event, false))
	{
		// then nothing's happening event-wise.
		return false;
	}
	else
	{
		// If the simulation actor caught the event,
		sim::Simulation & simulation = sim::Simulation::Ref();
		if (simulation.HandleEvent(event))
		{
			// then blank the event
			event_object = nullptr;
		}
		else
		{
			// else try and create an event object for script to deal with.
			event_object = create_event_object(event);	
		}
		
		// Either way, signal that there was activity.
		return true;
	}
}

PyMethodDef crag_methods[] = 
{
	{"time", time, METH_VARARGS, "Returns simulation time in seconds."},
	{"sleep", sleep, METH_VARARGS, "Sleeps the scripting thread without global-locking."},
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
: super(0x400)
, _source_file(nullptr)
{
	Assert(singleton == nullptr);
	singleton = this;

	smp::SetThreadPriority(1);
	smp::SetThreadName("Script");
	
	_source_file = fopen(_source_filename, "r");
	if (_source_file == nullptr)
	{
		std::cout << "Failed to open main Python file, \"" << _source_filename << "\".\n";
		return;
	}
	
    if (PyImport_AppendInittab("crag", & create_crag_module))
	{
		std::cout << "Failed to create crag python module.\n";
		return;
	}
    
	Py_Initialize();
}

script::ScriptThread::~ScriptThread()
{
	Py_Finalize();

	Assert(singleton == this);
	singleton = nullptr;

#if ! defined(NDEBUG)
	std::cout << "~ScriptThread: message buffer size=" << GetQueueCapacity() << std::endl;
#endif
}

// Note: Run should be called from same thread as c'tor/d'tor.
void script::ScriptThread::Run()
{
	PyRun_SimpleFileEx(_source_file, _source_filename, true);
	
	ProcessMessages();
	
	// TODO: Many many loose ends currently.
	//exit(0);
}

PyObject * script::ScriptThread::PollEvent()
{
	bool idle = true;
	
	while (true)
	{
		if (ProcessMessages() > 0)
		{
			idle = false;
		}
		
		PyObject * event_object;
		if (! handle_events(event_object))
		{	
			break;
		}
		
		if (event_object != nullptr)
		{
			return event_object;
		}
		
		idle = false;
	}
	
	if (idle)
	{
		// Yield for a little while
		smp::Sleep(0);
	}
	
	// There are no more pending events.
	Py_RETURN_NONE;
}

// TODO: Add documentation back in:
//Class<sim::Entity> entity_class("crag.Entity", * crag_module, "An Entity.", nullptr);
//Class<sim::Planet> planet_class("crag.Planet", * crag_module, "An Entity representing an astral body that has a surface.", nullptr, entity_class);
//Class<sim::Observer> observer_class("crag.Observer", * crag_module, "An Entity representing the camera.", observer_methods, entity_class);
//Class<sim::Star> star_class("crag.Star", * crag_module, "An Entity representing an astral body that emits light.", nullptr, entity_class);


char const * script::ScriptThread::_source_filename = "./script/main.py";

script::ScriptThread * script::ScriptThread::singleton = nullptr;
