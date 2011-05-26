//
//  intrusive_list.h
//  crag
//
//  Created by John McFarlane on 4/28/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace core
{
	
	// intrusive double-linked list classes
	namespace intrusive
	{
		
		// forward-declarations
		class hook;
		template <typename Class, hook Class::* Member> class list;
		
		
		// the intruder;
		// this hook class is added to the element class which is to be listed.
		// Instances of the element class can then be added to an intrusive::list.
		class hook
		{
		public:
			typedef size_t size_type;
			
			// functions
			hook() 
			{ 
				init(); 
			}
			
			~hook() 
			{ 
				deinit(); 
			}
			
			bool is_detached() const 
			{ 
				return _next == this; 
			}
			
			void detach()
			{
				if(! is_detached())
				{
					deinit();
					init();
				}
				
				Assert(is_detached());
				verify();
			}
			
			void verify() const
			{
#if defined(VERIFY)
				VerifyTrue((_next == this) == (_previous == this));
				VerifyRef(* _next);
				VerifyRef(* _previous);
#endif
			}
			
		protected:
			void init()
			{
				_next = _previous = this;
				Assert(is_detached());
				verify();
			}
			
			// insert this before l
			void insert(hook & l)
			{
				Assert(is_detached());
				_next = & l;
				_previous = l._previous;
				_next->_previous = _previous->_next = this;
				verify();
			}
			
			void deinit()
			{
				_next->_previous = _previous;
				_previous->_next = _next;
				verify();
			}
			
			// attributes
			hook * _next;
			hook * _previous;

			// friends
			template <typename Class, hook Class::* Member> friend class list;
		};
		
		
		// intrusive::list
		// A list which requires no allocation to contain members.
		template<typename Class, hook Class::* Member>
		class list : protected hook
		{
		public:
			typedef Class value_type;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;

			////////////////////////////////////////////////////////////////////////////////
			// d'tor
			
			~list()
			{
				unlink();
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// iterators
			
			class const_iterator
			{
				friend class list;
				const_iterator(hook & h) : _h(& h) 
				{ 
				}
			public:	
				typedef typename std::bidirectional_iterator_tag iterator_category;
				typedef typename list::value_type value_type;
				typedef typename list::difference_type difference_type;
				typedef typename list::value_type const * pointer;
				typedef typename list::value_type const & reference;
				
				const_iterator() : _h(nullptr) 
				{ 
				}
				
				const_iterator(const_iterator const & rhs) : _h(rhs._h) 
				{ 
				}
				
				bool operator==(const_iterator const & rhs) const 
				{ 
					return _h == rhs._h; 
				}
				bool operator!=(const_iterator const & rhs) const 
				{ 
					return _h != rhs._h; 
				}
				
				const_iterator operator++() 
				{ 
					_h = _h->_next; 
					return * this; 
				}
				const_iterator operator--() 
				{ 
					_h = _h->_previous; 
					return * this; 
				}
				
				value_type const & operator * () const
				{
					return ref(get_ptr());
				}
				value_type const * operator -> () const
				{
					return get_ptr();
				}
				
			protected:
				value_type * get_ptr() const
				{
					Assert(_h != nullptr);
					value_type * zclass_ptr = nullptr;
					hook & zmember_ptr = zclass_ptr->*Member;
					char * zmember_cptr = (char *)& zmember_ptr;
					char * member_cptr = (char *)_h;
					size_type offset = member_cptr - zmember_cptr;
					char * class_cptr = (char *)offset;
					value_type * class_ptr = (value_type *)class_cptr;
					return class_ptr;
				}
				
				// data
				hook * _h;
			};
			
			class iterator : public const_iterator
			{
				friend class list;
				iterator(hook & h) : const_iterator(h) 
				{ 
				}
			public:	
				iterator(const_iterator const & rhs) : const_iterator(rhs) 
				{ 
				}
				
				iterator operator++() 
				{ 
					const_iterator::operator++(); return * this; 
				}
				iterator operator--() 
				{ 
					const_iterator::operator--(); return * this; 
				}
				
				value_type & operator * () 
				{
					return ref(const_iterator::get_ptr());
				}
				value_type * operator -> () 
				{
					return const_iterator::get_ptr();
				}
			};
			
			iterator begin()
			{
				return iterator(* _next);
			}
			
			iterator end()
			{
				return iterator(* this);
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// capacity

			bool empty() const
			{
				return is_detached();
			}
			
			size_type size() const
			{
				size_type s = 0;
				for (hook * i = _next; i != this; i = i->_next)
				{
					++ s;
				}
				return s;
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// access

			value_type & front()
			{
				Assert(! empty());
				return * iterator(* _next);
			}
			value_type const & front() const
			{
				Assert(! empty());
				return * const_iterator(* _next);
			}
			
			value_type & back()
			{
				Assert(! empty());
				return * iterator(* _previous);
			}
			value_type const & back() const
			{
				Assert(! empty());
				return * const_iterator(* _previous);
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// modifiers
			
			void clear()
			{
				unlink();
				init();
			}
			
			void push_back(value_type & n)
			{
				hook & l = n.*Member;
				l.detach();
				l.insert(* this);
			}
			
			void pop_front()
			{
				_next->detach();
			}
			
			void pop_back()
			{
				_previous->detach();
			}
			
			// insert insertion before position
			void insert(iterator position, value_type & insertion)
			{
				hook & l = insertion.*Member;
				l.detach();

				insert_detached(position, insertion);
			}
			
			// same as insert but insertion MUST be detatched already
			void insert_detached(iterator position, value_type & detached_insertion)
			{
				// TODO: Test that position is contained and insertion is not.
				// TODO: find isn't going to work because end is sort-of legit.
				hook & insertion_hook = detached_insertion.*Member;
				Assert(insertion_hook.is_detached());
				
				value_type & position_element = * position;
				hook & position_hook = position_element.*Member;
				
				insertion_hook.insert(position_hook);
			}
			
		private:
			void unlink()
			{
				hook * i = _next;
				while (true)
				{
					hook * n = i->_next;
					if (i == n)
					{
						break;
					}
					else
					{
						n->init();
					}
				}
			}
		};
	}
}
