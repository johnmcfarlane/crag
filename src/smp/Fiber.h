//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "FiberInterface.h"

#include <ucontext.h>


// Note: Fibers are very much NOT a symetric multi-processing feature
// but they are intended to be used in conjunction with the scheduler.
namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// Fiber - lightweight thread
	
	class Fiber : public FiberInterface
    {
        ////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		typedef int (* Function)(void * data);
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber(std::size_t stack_size = SIGSTKSZ - MINSIGSTKSZ);
		~Fiber();
		
		// true iff the fiber has been launched and had not quit or been killed 
		bool IsRunning() const;
        
		// Launches the fiber with the given functor;
		// Functor takes a FiberInterface as its sole input parameter.
		template <typename FUNCTOR> void Launch(FUNCTOR functor);

		// Continues execution of the fiber.
		void Continue();
        
	private:
        class FunctorWrapperBase;
        template <typename FUNCTOR> class FunctorWrapper;

		// Creates and launches a new fiber.
		void Launch(FunctorWrapperBase & functor_wrapper);
		
		static void OnLaunch(Fiber * fiber);
		
		// FiberInterface overrides
		virtual void Yield() override;
		virtual void Kill() override;
		
        ////////////////////////////////////////////////////////////////////////////////
		// variables
		ucontext_t _context;
		void * _stack;
		FunctorWrapperBase * _functor_wrapper;
    };
	

	////////////////////////////////////////////////////////////////////////////////
	// Fiber member definitions
	
	// polymorphic class of things to be called on fiber launch
	class Fiber::FunctorWrapperBase
	{
	public:
		virtual ~FunctorWrapperBase() = default;
		virtual void OnLaunch (FiberInterface &) = 0;
	};

	// templatized class of thing to be called on fiber launch
	template <typename FUNCTOR>
	class Fiber::FunctorWrapper : public Fiber::FunctorWrapperBase
	{
	public:
		FunctorWrapper(FUNCTOR & functor) : _functor(functor) { }
	private:
		virtual void OnLaunch (FiberInterface & fiber_interface) { _functor(fiber_interface); }
		FUNCTOR _functor;
	};
	
	template <typename FUNCTOR>
	void Fiber::Launch(FUNCTOR functor)
	{
		FunctorWrapperBase * functor_wrapper = new FunctorWrapper<FUNCTOR>(functor);
		Launch(ref(functor_wrapper));
	}
}
