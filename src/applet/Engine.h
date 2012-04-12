//
//  Engine.h
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletBase.h"

#include "smp/Daemon.h"

#include "core/Singleton.h"


namespace applet
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	
	template <typename FUNCTOR> class Applet;
	
	////////////////////////////////////////////////////////////////////////////////
	// definitions

	typedef smp::Uid Uid;
	
	// applet::Daemon type
	class Engine;
	typedef smp::Daemon<Engine> Daemon;
	
	// The applet scheduling is coordinated from here.
	// When Run finishes, the program is done.
	class Engine
	{
		OBJECT_SINGLETON(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

	public:
		typedef smp::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();

		// given a UID, returns the applet associated with it
		AppletBase * GetObject(Uid uid);
		
		// daemon messages
		template <typename FUNCTOR = void (*) (AppletInterface & applet_interface)>
		void Launch(FUNCTOR & functor)
		{
			AppletBase * object = new Applet<FUNCTOR> (functor);
			OnAddObject(object);
		}
		
		void OnQuit();
		
		template <typename OBJECT_TYPE>
		void OnCreateObject(Uid const & uid)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE ();
			object->SetUid(uid);
			OnAddObject(object);
		}
		
		void OnAddObject(AppletBase * const & entity);
		void OnRemoveObject(Uid const & uid);
		
		void SetQuitFlag();
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
	private:
		bool HasFibersActive() const;
		
		bool ProcessTasks();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// sim::Engine time.
		Time _time;
		
		// Collection of all active applets
		AppletBase::List _applets;
		
		bool _quit_flag;
	};
}