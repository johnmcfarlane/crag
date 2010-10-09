/*
 *  SimpleMutex.cpp
 *  crag
 *
 *  Created by John on 8/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "SimpleMutex.h"


smp::SimpleMutex::SimpleMutex()
{
	flag[0] = false;
	flag[1] = false;
	turn = 0;
}

smp::SimpleMutex::~SimpleMutex()
{
	Assert(! flag[0]);
	Assert(! flag[1]);
	Assert(turn == 0 || turn == 1);
}

void smp::SimpleMutex::Lock(int i)
{
	Assert(i == 0 || i == 1);
	Assert(! flag[i]);
	
	flag[i] = true;
	while (flag[! i] == true)
	{
		if (turn != i)
		{
			flag[i] = false;
			while (turn != i)
			{
			}
			flag[i] = true;
		}
	}
}

void smp::SimpleMutex::Unlock(int i)
{
	Assert(i == 0 || i == 1);
	Assert(flag[i]);
	
	turn = ! i;
	flag[i] = false;
}
