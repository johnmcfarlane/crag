/*
 *  DebugUtils.cpp
 *  Crag
 *
 *  Created by John on 2/10/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "core/debug.h"


////////////////////////////////////////////////////////////////////////////////
// DumpStream members

#if defined(DUMP)

DumpStream::DumpStream(std::ostream & _out)
: out(_out)
{
	strcpy(indent, "\n");
	Assert(strlen(indent) + 1 < max_indent);
}

DumpStream::DumpStream(DumpStream & previous)
: out(previous.out)
{
	sprintf(indent, "%s\t", previous.indent);
	Assert(strlen(indent) + 1 < max_indent);
}

char const * DumpStream::NewLine() const
{
	return indent;
}

#endif
