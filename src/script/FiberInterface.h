//
//  FiberInterface.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//


namespace script
{
	// forward-declare
	class Condition;
	
	// Interface to the Fiber class;
	// This interface is passed to a Fiber's function object.
	class FiberInterface
	{
	public:
		// functions
		virtual ~FiberInterface() { }
		
		// quit flag get/setter
		virtual bool GetQuitFlag() const = 0;
		virtual void SetQuitFlag() = 0;

		// pause execution in various ways
		virtual void Yield() = 0;
		virtual void Sleep(Time duration) = 0;
		virtual void Wait(Condition & condition) = 0;

		// launch another fiber
		template <typename FUNCTOR>
		void Launch(FUNCTOR const & functor);
		
	private:
		virtual void LaunchFiber(class Fiber & fiber) = 0;
	};
}