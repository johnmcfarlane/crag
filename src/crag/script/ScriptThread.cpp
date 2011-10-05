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

#include "sim/axes.h"
#include "sim/Simulation.h"

#include "gfx/Renderer.h"


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
	sim::Simulation & simulation = sim::Simulation::Daemon::Ref();
	
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
	script::ScriptThread & script_thread = script::ScriptThread::Daemon::Ref();
	return script_thread.PollEvent();
}

PyObject * set_camera(PyObject * /*self*/, PyObject * args)
{
	typedef sim::Vector3 Vector;

	// Get the coordinates.
	sim::Vector3 position;
	if (! PyArg_ParseTuple(args, "ddd", 
						   & position.x, & position.y, & position.z))
	{
		std::cout << "set_camera error: invalid inputs. Must be x, y, z numbers." << std::endl;
	}
	else
	{
		sim::SetCameraMessage message;
		message.transformation = sim::Transformation(position);
		gfx::Renderer::Daemon::SendMessage(message);
	}
	
	Py_RETURN_NONE;
}


////////////////////////////////////////////////////////////////////////////////
// support functions

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

PyMethodDef crag_methods[] = 
{
	{"time", time, METH_VARARGS, "Returns simulation time in seconds."},
	{"sleep", sleep, METH_VARARGS, "Sleeps the scripting thread without global-locking."},
	{"get_event", get_event, METH_VARARGS, "Returns the next event in the event queue."},
	{"set_camera", set_camera, METH_VARARGS, "Sets the origin to the given (x,y,z) position."},
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
: _source_file(nullptr)
, _message_queue(nullptr)
{
	smp::SetThreadPriority(1);
	smp::SetThreadName("Script");
	
	_source_file = fopen(_source_filename, "r");
	if (_source_file == nullptr)
	{
		std::cerr << "Failed to open main Python file, \"" << _source_filename << "\"." << std::endl;
		return;
	}
	
    if (PyImport_AppendInittab("crag", & create_crag_module))
	{
		std::cerr << "Failed to create crag python module." << std::endl;
		return;
	}
	
	// Set executable name so Python knows where to looks for libs.
	wchar_t program_path[FILENAME_MAX + 1];
	mbstowcs(program_path, sys::GetProgramPath(), FILENAME_MAX);
	Py_SetProgramName(program_path);
	
	Py_InitializeEx(0);
}

script::ScriptThread::~ScriptThread()
{
	Py_Finalize();
}

void script::ScriptThread::OnMessage(EventMessage const & message)
{
	PyObject * event_object = create_event_object(message.event);
	if (event_object == nullptr)
	{
		return;
	}
	
	_events.push(event_object);
}

void script::ScriptThread::OnMessage(smp::TerminateMessage const & message)
{
}

// Note: Run should be called from same thread as c'tor/d'tor.
void script::ScriptThread::Run(Daemon::MessageQueue & message_queue)
{
	_message_queue = & message_queue;
	
#if defined(WIN32)
	if (! RedirectPythonOutput())
	{
		std::cerr << "Failed to redirect python output" << std::endl;
	}
#endif

	PyRun_SimpleFileEx(_source_file, _source_filename, true);
	
	_message_queue->DispatchMessages(* this);
	_message_queue = nullptr;
}

PyObject * script::ScriptThread::PollEvent()
{
	bool idle = _message_queue->DispatchMessages(* this) == 0;
	
	if (_events.empty())
	{
		if (idle)
		{
			// Yield for a little while
			smp::Yield();
		}

		// There are no more pending events.
		Py_RETURN_NONE;
	}
	
	PyObject * event_object = _events.front();
	_events.pop();
	
	return event_object;
}

bool script::ScriptThread::RedirectPythonOutput()
{
#if defined(WIN32)
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* io = PyImport_ImportModule("io");

	PyObject* pystdout = PyObject_CallMethod(io, "open", "ss", "pout.txt", "wt");
	if (-1 == PyObject_SetAttrString(sys, "stdout", pystdout)) 
	{
		return false;
	}
	Py_DECREF(pystdout);

	PyObject* pystderr = PyObject_CallMethod(io, "open", "ss", "perr.txt", "wt");
	if (-1 == PyObject_SetAttrString(sys, "stderr", pystderr)) 
	{
		return false;
	}
	Py_DECREF(pystderr);

	Py_DECREF(io);
	Py_DECREF(sys);
#endif
	
	return true;
}


char const * script::ScriptThread::_source_filename = "./script/main.py";
