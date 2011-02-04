/*
 *  Script.h
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace script
{
	
	class Script
	{
	public:
		Script();
		Script(std::istream & source_file);
		Script(char const * source_filename);
		~Script();
		
		void Clear();
		void Load(std::istream & source_file);
		bool Load(char const * source_code);

		bool IsOk() const;
		char const * GetBuffer() const;
		
	private:
		char * buffer;
	};

}
