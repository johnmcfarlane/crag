//
//  StackCushion.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace script
{
	// Provides a gap between the top of the stack used by ScriptThread 
	// and the bottom of the stack used by Fibers. Also checks that
	// a portion of the gap remains intacts as a safety measure.
	class StackCushion
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		StackCushion();
		
		bool IsSafe() const;
		
	private:
		char * SafeMarginBegin();
		char const * SafeMarginBegin() const;
		
		char * SafeMarginEnd();
		char const * SafeMarginEnd() const;
		
		////////////////////////////////////////////////////////////////////////////////
		// constants
		
		// A multiplier which accounts for the fact that more stack is used in debug builds.
		// The size of the cushion in bytes.
		static size_t const _cushion_size = 2048 * sizeof(void *);
		
		// The size of the area of the cushion intended to remain untouched.
		static size_t const _safe_margin_size = _cushion_size >> 1;
		
		// The value written into the safe margin.
		static char const _marker_value = (char)(0x9d);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		char _cushion_buffer[_cushion_size];
	};
}
