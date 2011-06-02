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
	
	// Intrusive Double-Linked List class
	namespace intrusive
	{
		
		// forward-declarations
		template <typename Class> class list_base;
		
		
		////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////
		// core::intrusive::hook
		// 
		// To enable listing of a Class, give it a member variable of type 
		// core::intrusive::hook<Class>. Then add them to a list of type, 
		// core::intrusive::hook<Class, & Class::MemberVariable>. Objects can
		// only be listed multiple times if they have multiple hooks.
		
		template<typename Class>
		class hook
		{
		public:
			typedef size_t size_type;
			typedef Class value_type;
			
			// functions
			hook() 
			{ 
				init(); 
			}
			
			~hook() 
			{ 
				// Must call detach before attached hook is destroyed.
				Assert(_next == _previous);
			}
			
			bool is_attached() const 
			{ 
				return _next != nullptr; 
			}
			
			bool is_detached() const 
			{ 
				return _next == nullptr; 
			}
			
			value_type * get_next()
			{
				return _next;
			}
			
			value_type * get_previous()
			{
				return _previous;
			}
			
			// not to be called on a list node
			void verify() const
			{
#if defined(VERIFY)
				VerifyTrue((_next == nullptr) == (_previous == nullptr));
				VerifyTrue((_next == nullptr) == (_previous == _next));
				VerifyPtr(_next);
				VerifyPtr(_previous);
#endif
			}
			
		protected:
			void init()
			{
				_next = _previous = nullptr;
				
				Assert(is_detached());
				verify();
			}

			// variables
			value_type * _next;
			value_type * _previous;
			
			// friends
			friend class list_base<Class>;
		};
		
		
		////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////
		// core::intrusive::list_base - do not use; use list instead.
		
		template <typename Class>
		class list_base
		{
		public:
			typedef ptrdiff_t difference_type;
			typedef hook<Class> hook_type;
			typedef size_t size_type;
			typedef Class value_type;
			
			////////////////////////////////////////////////////////////////////////////////
			// iterators
			
			// TODO: Add parameters, value_type * hook_type::*Next, value_type * hook_type::*Previous.
			// TODO: This will allow iterators to become reverse_iterators easily.
			template <hook_type Class::*Member>
			class const_iterator
			{
				//friend class list;
			public:	
				typedef typename std::bidirectional_iterator_tag iterator_category;
				typedef  Class value_type;
				typedef typename list_base::difference_type difference_type;
				typedef typename list_base::value_type const * pointer;
				typedef typename list_base::value_type const & reference;
				
				const_iterator() 
				: _pos(nullptr) 
				{ 
				}
				const_iterator(const_iterator const & rhs) 
				: _pos(rhs._pos) 
				{ 
				}
				const_iterator(value_type const * rhs) 
				: _pos(const_cast<value_type *>(rhs)) 
				{ 
				}
				
				bool operator==(const_iterator const & rhs) const 
				{ 
					return _pos == rhs._pos; 
				}
				bool operator!=(const_iterator const & rhs) const 
				{ 
					return _pos != rhs._pos; 
				}
				
				const_iterator operator++() 
				{ 
					hook_type & h = _pos->*Member;
					_pos = h._next; 
					return * this; 
				}
				const_iterator operator--() 
				{ 
					hook_type & h = _pos->*Member;
					_pos = h._previous; 
					return * this; 
				}
				
				value_type const & operator * () const
				{
					return ref(_pos);
				}
				value_type const * operator -> () const
				{
					return _pos;
				}
				
			protected:
				
				// data
				Class * _pos;
			};
			
			template <hook_type Class::*Member>
			class iterator : public const_iterator<Member>
			{
				//friend class list;
				typedef const_iterator<Member> super;
			public:	
				iterator(super const & rhs) 
				: super(rhs) 
				{ 
				}
				iterator(value_type * h) 
				: super(h) 
				{ 
				}
				
				iterator operator++() 
				{ 
					super::operator++(); 
					return * this; 
				}
				
				iterator operator--() 
				{ 
					super::operator--(); 
					return * this; 
				}
				
				value_type & operator * () 
				{
					return ref(super::_pos);
				}
				
				value_type * operator -> () 
				{
					return super::_pos;
				}
			};
			
			template <hook_type Class::*Member/*, value_type * hook_type::*Next, value_type * hook_type::*Previous*/>
			iterator<Member/*, Next, Previous*/> begin()
			{
				return iterator<Member/*, & hook_type::_next, & hook_type::_previous*/>(_head.get_next());
			}
			
			template <hook_type Class::*Member/*, value_type * hook_type::*Next, value_type * hook_type::*Previous*/>
			iterator<Member/*, Next, Previous*/> end()
			{
				Class & o = get_object<Member>(_head);
				return iterator<Member>(& o);
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// access
			
			template <hook_type Class::*Member>
			bool contains(value_type const & element)
			{
				return std::find(begin(), end(), element) != end();
			}
			
			// detach an element that is in this list.
			// TODO: Break out static section and move this-based Asserts into list.
			template <hook_type Class::*Member>
			void remove(value_type & element)
			{
				Assert(is_attached(element));
				Assert(contains(element));
				
				unlink<Member>(element);
				
				Assert(is_detached(element));
				Assert(! contains(element));
			}
			
			template <hook_type Class::*Member>
			void init()
			{
				value_type & sham = get_object<Member>(_head);
				_head._next = _head._previous = & sham;
			}
			
			////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////
			// helpers
			
			template <hook_type Class::*Member>
			static void insert_detached(value_type & next, value_type & insertion)
			{
				// Check that values of insertion's pointers are unimportant.
				hook_type & insertion_hook = insertion.*Member;
				Assert(insertion_hook.is_detached());
				
				// Check that insertion point is valid.
				hook_type & next_hook = next.*Member;
				Assert(next_hook._previous != nullptr);
				Assert(next_hook._next != nullptr);
				
				// Get the element that belongs before the insertion point.
				value_type & previous = * next_hook._previous;
				hook_type & previous_hook = previous.*Member;
				
				// Set insertion's pointers.
				insertion_hook._next = & next;
				insertion_hook._previous = & previous;
				
				// Point the neighbours at the insertion.
				previous_hook._next = & insertion;
				next_hook._previous = & insertion;
			}
			
			template <hook_type Class::*Member>
			void unlink_all()
			{
				value_type * i = _head._next;
				while (true)
				{
					hook_type & h = i->*Member;
					if (& h == & _head)
					{
						break;
					}
					
					i = h._next;
					h.init();
				}
				
				// All removed elements are in a good state
				// but the _head element is in a bad state.
			}
			
			// Removes attached from its current list
			// but leaves its links in a bad state.
			template <hook_type Class::*Member>
			static void unlink(value_type & attached)
			{
				hook_type & attached_hook = attached.*Member;
				Assert(attached_hook.is_attached());
				
				value_type & next = * attached_hook._next;
				value_type & previous = * attached_hook._previous;
				
				hook_type & next_hook = next.*Member;
				hook_type & previous_hook = previous.*Member;
				
				next_hook._previous = & previous;
				previous_hook._next = & next;
			}
			
			// Removes attached from its current list
			// and fixes is up.
			template <hook_type Class::*Member>
			static void detach(value_type & attached)
			{
				unlink<Member>(attached);
				
				hook_type & h = attached.*Member;
				h.init();
			}
			
			// TODO: Can't these just in list and maybe succeed in making list a friend of hook?
			template <hook_type Class::* Member>
			static hook_type & get_hook(Class & o)
			{
				return o.*Member;
			}
			
			template <hook_type Class::* Member>
			static value_type & get_object(hook_type & h)
			{
				VerifyRef(h);
				
				// Get byte pointer to where hook would be in a null object.
				value_type * const null_object_ptr = nullptr;
				char * null_hook_cptr = (char *)&(null_object_ptr->*Member);
				
				// Get byte pointer to where it is now.
				char * hook_cptr = (char *)& h;
				
				// Get the difference between the two pointers.
				// This is equal to the byte pointer of the object which h is part of.
				// (Hopefully, this is the first line that actually makes code.)
				size_t offset = hook_cptr - null_hook_cptr;
				
				// Return 
				value_type & object_ref = * reinterpret_cast<Class *>(offset);
				//Assert(& get_hook<Class, Member>(object_ref) == & h);
				
				return object_ref;
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// data
			
			// Note: detachment/attachment are different on this object.
			// is_detached/is_attached should not be used on this object.
			hook_type _head;
		};
		
		
		////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////
		// core::intrusive::list
		//
		// Stores elements of Class where Class contains a member variable of type,
		// core::intrusive::hook<Class> which is specified by parameter, Member.
		// Warning: Elements must be explicitly removed from one list 
		// before being added to another.
		
		template<typename Class, hook<Class> Class::* Member>
		class list : public list_base<Class>
		{
			////////////////////////////////////////////////////////////////////////////////
			// types
			
			typedef typename core::intrusive::list_base<Class> super;
		public:
			typedef typename super::hook_type hook_type;
			typedef typename super::size_type size_type;
			typedef typename super::value_type value_type;
			
			typedef typename ::core::intrusive::list_base<Class>::template const_iterator<Member> const_iterator;
			typedef typename ::core::intrusive::list_base<Class>::template iterator<Member> iterator;
			
			////////////////////////////////////////////////////////////////////////////////
			// d'tor
			
			list()
			{
				super::template init<Member>();
			}
			
			~list()
			{
				super::template unlink_all<Member>();
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// iterators
			
			iterator begin()
			{
				return super::template begin<Member>();
			}
			iterator end()
			{
				return super::template end<Member>();
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// capacity
			
			bool empty() const
			{
				return super::_head.is_detached();
			}
			
			size_type size() const
			{
				size_type s = 0;
				for (const_iterator i = begin(); i != end(); ++ i)
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
				return * super::_head.get_next();
			}
			
			value_type const & front() const
			{
				Assert(! empty());
				return * super::_head.get_next();
			}
			
			value_type & back()
			{
				Assert(! empty());
				hook_type & h = super::_head;
				return * h.get_previous();
			}
			value_type const & back() const
			{
				Assert(! empty());
				return super::_head._previous;
			}
			
			////////////////////////////////////////////////////////////////////////////////
			// modifiers
			
			void push_front(value_type & n)
			{
				value_type & previous = * super::_head.get_next();
				super::template insert_detached<Member>(previous, n);
			}
			void push_back(value_type & n)
			{
				value_type & next = super::template get_object<Member>(super::_head);
				super::template insert_detached<Member>(next, n);
			}
			
			void pop_front()
			{
				Assert(! empty());
				super::template detach<Member>(* super::_head.get_next());
			}
			void pop_back()
			{
				Assert(! empty());
				super::template detach<Member>(* super::_head.get_previous());
			}
			
			// insert insertion before position
			void insert(iterator position, value_type & insertion)
			{
				super::template insert_detached<Member>(* position, insertion);
			}
			
			void clear()
			{
				super::template unlink_all<Member>();
				super::template init<Member>();
			}
		};
		
	}
}
