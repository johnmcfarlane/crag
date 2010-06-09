/*
 *  ProfileEntry.h
 *  Crag
 *
 *  Created by john on 6/08/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#if defined(PROFILE)

#include <SDL_timer.h>


namespace profile
{

	class Entry
	{
	public:
		Entry(char const * tag, int max_entries = 0);
		~Entry();

		void OnEnter()
		{
			Assert(num_entries >= 0 && num_entries != max_entries);
			Uint32 time = SDL_GetTicks();
			sum_time += num_entries * (time - last_time);
			last_time = time;
			++ num_entries;
		}

		void OnExit()
		{
			Uint32 time = SDL_GetTicks();
			sum_time += num_entries * (time - last_time);
			last_time = time;
			-- num_entries;
			Assert(num_entries >= 0 && num_entries != max_entries);
		}

	private:
		Entry * next;
		char const * tag;
		int num_entries;
		int max_entries;
		Uint32 last_time;
		Uint32 sum_time;
	};

	class Scope
	{
	public:
		Scope(Entry & init_entry)
		: entry(init_entry)
		{
			entry.OnEnter();
		}

		~Scope()
		{
			entry.OnExit();
		}

	private:
		Entry & entry;
	};
}

#define PROFILE_BEGIN(TAG) \
	static profile::Entry profile_entry_##TAG(TAG); \
	profile_entry_##TAG.OnEnter()

#define PROFILE_END(TAG) \
		profile_entry_##TAG.OnEnter()

#define PROFILE_SCOPE(TAG) \
	static profile::Entry profile_entry_##TAG(TAG); \
	profile::Scope scope(profile_entry_##TAG)

#else

#define PROFILE_BEGIN(TAG) DO_NOTHING
#define PROFILE_END(TAG) DO_NOTHING
#define PROFILE_SCOPE(TAG) DO_NOTHING

#endif	// defined(PROFILE)
