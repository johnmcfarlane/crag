//
//  StackFrameBuffer.h
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace script
{
	// Storage for a copy of part of the stack.
	// Used to put fibers on ice when they're swapped out.
	class StackFrameBuffer
	{
		////////////////////////////////////////////////////////////////////////////////
		// constants
		
		enum
		{
			// ensures that indices into PointerRange are address value-sorted
#if (STACK_GROWTH_DIRECTION == 1)
			bottom_index = 0,
			top_index = 1,
#elif (STACK_GROWTH_DIRECTION == -1)
			top_index = 0,
			bottom_index = 1,
#endif
			max_index	// 2
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
	public:
		typedef Vector<char *, max_index> PointerRange;
		typedef Vector<char const *, max_index> ConstPointerRange;
		typedef std::vector<char> StackBuffer;
		
		// more constants (following prerequisite type definition)
		static char * const null;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		StackFrameBuffer();
		~StackFrameBuffer();
		
		// Note: on x86 - and other - architectures 
		// where stack pointer decreases with growth, bottom > top.
		void SetBottom(char * bottom);
		void SetTop(char * top);
		
		// Given the current program state, estimate there the top of the stack is.
		static char * CalculateTop();
		
		// Take a snapshot of the section of the stack given by the range.
		void Copy();
		
		// Copy the stored snapshot back to its source.
		void Restore() const;
		
		template <typename OBJECT>
		bool IsPersistent(OBJECT const & object) const
		{
			ConstPointerRange data;
			data[0] = reinterpret_cast<char const *>(& object);
			data[1] = reinterpret_cast<char const *>((& object) + 1);
			return IsPersistent(data);
		}
		
		// returns true iff the given range is OK to refer to outside the fiber.
		// (Obviously, data could still contain pointers into non-persistent area.)
		bool IsPersistent(ConstPointerRange const & data) const;
		
#if defined(VERIFY)
		void Verify() const;
#endif
	private:
		std::size_t GetRangeBytes() const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		PointerRange _range;
		StackBuffer _buffer;
	};
}
