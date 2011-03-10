/*
 *  ptr.h
 *  crag
 *
 *  Created by John McFarlane on 2/1/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Mutex.h"


namespace smp
{
	// helper class for ptr
	template <typename T>
    struct ptr_ref
    {
		T * _element;
		Mutex * _mutex;
		
		explicit ptr_ref(T * e, Mutex * m)
		: _element(e), _mutex(m) 
		{ 
		}
    };
	
	
	// Definition of class ptr which is heavily 'inspired' by 
	// a gcc 4.2 implementation of auto_ptr. But instead of 
	// managing the deallocation of the T element, it manages
	// the mutex locking.
	template <typename T>
    class ptr
    {
    private:
		T * _element;
		Mutex * _mutex;
		
    public:
		/// The pointed-to type.
		typedef T element_type;

		// c'tors
		ptr() throw() 
		: _element(nullptr), _mutex(nullptr) 
		{ 
		}
		
		explicit ptr(element_type & e, Mutex & m) throw() 
		: _element(& e), _mutex(& m) 
		{ 
			m.Lock();
		}
		
		ptr(ptr & that) throw() 
		: _element(that.release_e()), _mutex(that.release_m()) 
		{ 
		}
		
		template <typename T2> 
		ptr(ptr<T2> & that) throw() 
		: _element(that.release_e()), _mutex(that.release_m()) 
		{ 
		}
		
		~ptr() 
		{ 
			clear(); 
		}
		
		// assignment
		ptr & operator=(ptr & that) throw()
		{
			set(that.release_e(), that.release_m());
			return * this;
		}
		
		template <typename T2> 
		ptr & operator=(ptr<T2> & that) throw()
        {
			set(that.release_e(), that.release_m());
			return * this;
		}
		
		// dereference
		element_type & operator*() const throw() 
		{
			Assert(_element != nullptr);
			return * _element; 
		}
		
		element_type * operator->() const throw() 
		{
			Assert(_element != nullptr);
			return _element; 
		}
		
	private:
		// invasion!
		element_type * get() const throw() 
		{ 
			return _element; 
		}
		
		element_type * release_e() throw()
		{
			element_type * tmp = _element;
			_element = nullptr;
			return tmp;
		}
		
		Mutex * release_m() throw()
		{
			Mutex * tmp = _mutex;
			_mutex = nullptr;
			return tmp;
		}
		
		void clear() throw()
		{
			Assert((_mutex == nullptr) == (_element == nullptr));
			if (_mutex != nullptr)
			{
				_mutex->Unlock();
				_element = nullptr;
				_mutex = nullptr;
			}
		}		
		
		void set(element_type & e /*= nullptr*/, Mutex & m) throw()
		{
			clear();
			_element = & e;
			_mutex = & m;
		}
		
	public:
		ptr(ptr_ref<element_type> ref) throw()
		: _element(ref._element), _mutex(ref._mutex) 
		{ 
		}
		
		ptr & operator=(ptr_ref<element_type> ref) throw()
		{
			set(ref._element, ref._mutex);
			return * this;
		}
		
		template <typename T2>
        operator ptr_ref<T2>() throw()
        { return ptr_ref<T2>(this->release_e(), this->release_m()); }
		
		template<typename T2>
        operator ptr<T2>() throw()
        { return ptr<T2>(this->release_e(), this->release_m()); }
	};

}
