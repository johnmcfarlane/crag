/*
 *  Profile.cpp
 *  Crag
 *
 *  Created by john on 6/08/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "profile.h"


namespace profile
{
	namespace
	{
		Entry * list_head = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Entry definitions

	Entry::Entry(char const * init_tag, int init_max_entries)
	{
		Assert(init_tag != nullptr);
		Assert(init_max_entries > 0);

		next = list_head;
		tag = tag;
		num_entries = nullptr;
		max_entries = init_max_entries;
		sum_time = 0;
		list_head = this;
	}

	Entry::~Entry()
	{
		Assert(next != nullptr);
		Assert(tag != nullptr);
		Assert(num_entries == 0);
	}
}
