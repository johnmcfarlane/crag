//
//  function_ref.h
//  crag
//
//  Created by John McFarlane on 2012-08-17.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace core
{
	////////////////////////////////////////////////////////////////////////////////
	// function_ref
	//
	// like std::function, but refers to a function,
	// rather than holding a dynamically allocated copy of a function

	template <typename FUNCTION>
	class function_ref;
	
	template <typename RESULT, typename ... PARAMETERS>
	class function_ref <RESULT(PARAMETERS ...)>
	{
		// types
		typedef RESULT (* dispatch_type)(void *, PARAMETERS ...);

	public:
		// functions
		function_ref(function_ref const & rhs)
		: _dispatch(rhs._dispatch)
		, _functor_ptr(rhs._functor_ptr)
		{
		}
		
#if defined(WIN32)
		// TODO: bugs in VC++ compiler mean copy c'tor is skipped and function_ref is itself wrapped in a function_ref.
		template <typename FUNCTOR>
#else
		template <typename FUNCTOR, typename std::enable_if<! std::is_same<FUNCTOR, function_ref>::value, FUNCTOR>::type* dummy = nullptr>
#endif
		function_ref(FUNCTOR & functor)
		: _dispatch(& dispatch<FUNCTOR>)
		, _functor_ptr(reinterpret_cast<void *>(& functor))
		{
			static_assert(sizeof(& functor) == sizeof(& _functor_ptr), "implementation not supported");
		}
		
		bool operator==(function_ref const & rhs) const
		{
			return _dispatch == rhs._dispatch
				&& _functor_ptr == rhs._functor_ptr;
		}
		
		bool operator!=(function_ref const & rhs) const
		{
			return ! operator==(rhs);
		}
		
		RESULT operator()(PARAMETERS ... parameters) const
		{
			return (*_dispatch)(_functor_ptr, parameters ...);
		}
		
	private:
		template <typename FUNCTOR>
		static RESULT dispatch(void * functor_ptr, PARAMETERS ... parameters)
		{
			auto & functor = * reinterpret_cast<FUNCTOR *>(functor_ptr);
			return (functor)(parameters ...);
		}
		
		// variables
		dispatch_type _dispatch;	// pointer to specialization of dispatch
		void * _functor_ptr;	// pointer to functor passed to constructor
	};
}
