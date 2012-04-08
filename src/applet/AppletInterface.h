//
//  AppletInterface.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//


namespace applet
{
	// forward-declare
	class Condition;
	class AppletBase;
	
	// Interface to the Applet class;
	// This interface is passed to a Applet's function object.
	class AppletInterface
	{
	public:
		// functions
		virtual ~AppletInterface() { }
		
		// script thread quit flag get/setter
		virtual bool GetQuitFlag() const = 0;
		virtual void SetQuitFlag() = 0;

		// pause execution in various ways
		virtual void Yield() = 0;
		virtual void Sleep(Time duration) = 0;
		virtual void Wait(Condition & condition) = 0;
		
		// launch another fiber (and deletes the script afterwards)
		virtual void Launch(AppletBase & script) = 0;
	};
}
