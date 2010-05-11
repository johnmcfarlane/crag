/*
 *  DebugUtils.cpp
 *  Crag
 *
 *  Created by John on 2/10/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "core/debug.h"


////////////////////////////////////////////////////////////////////////////////
// DumpStream members

#if DUMP

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
