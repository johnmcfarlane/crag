//
//  form/QuaternaBuffer.cpp
//  crag
//
//  Created by John on 3013-11-05.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "QuaternaBuffer.h"

#include "Quaterna.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// QuaternaBuffer member definitions

QuaternaBuffer::QuaternaBuffer(int max_num_quaterne)
: _quaterne(new Quaterna [max_num_quaterne])
, _quaterne_sorted_end(_quaterne)
, _quaterne_used_end(_quaterne)
, _quaterne_end(_quaterne + max_num_quaterne)
{
}

QuaternaBuffer::~QuaternaBuffer()
{
#if ! defined(NDEBUG)
	for (Quaterna const * i = _quaterne; i != _quaterne_used_end; ++ i)
	{
		ASSERT(! i->HasGrandChildren());
		ASSERT(! i->nodes[0].IsInUse());
		ASSERT(! i->nodes[1].IsInUse());
		ASSERT(! i->nodes[2].IsInUse());
		ASSERT(! i->nodes[3].IsInUse());
	}
#endif

	delete _quaterne;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(QuaternaBuffer, self)
	CRAG_VERIFY_ARRAY_POINTER(self._quaterne_sorted_end, self._quaterne, self._quaterne_end);
	CRAG_VERIFY_ARRAY_POINTER(self._quaterne_used_end, self._quaterne, self._quaterne_end);
	CRAG_VERIFY_ARRAY_POINTER(self._quaterne_end, self._quaterne, self._quaterne_end);
	
	CRAG_VERIFY_OP(self._quaterne_sorted_end, >=, self._quaterne);
	CRAG_VERIFY_OP(self._quaterne_used_end, >=, self._quaterne_sorted_end);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void QuaternaBuffer::Clear()
{
	_quaterne_sorted_end = _quaterne_used_end = _quaterne;
}

void QuaternaBuffer::UpdateScores()
{
	ForEach([] (Quaterna & q) 
	{
		Node * parent = q.nodes[0].GetParent();
		q.parent_score = (parent != nullptr) ? parent->score : -1;
	}, 1024, true);
	
	// This basically says: "as far as I know, none of the quaterne are sorted."
	_quaterne_sorted_end = _quaterne;
}

// Algorithm to sort nodes array. 
void QuaternaBuffer::Sort()
{
	if (_quaterne_sorted_end == _quaterne_used_end)
	{
		CRAG_VERIFY(* this);
		return;
	}

	std::sort(_quaterne, _quaterne_used_end, [] (Quaterna const & lhs, Quaterna const & rhs) {
		return lhs.parent_score > rhs.parent_score;
	});
	
	_quaterne_sorted_end = _quaterne_used_end;
}

// Find the (known) lowest-scoring quaterna in used.
float QuaternaBuffer::GetLowestSortedScore() const
{
	auto lowest_sorted = GetLowestSorted();
	
	// if there is no such thing,
	if (! lowest_sorted) 
	{
		// return a value that can't fail in a test for replacement
		return std::numeric_limits<float>::max();
	}
	
	return lowest_sorted->parent_score;
}

// Find the (known) lowest-scoring quaterna in used.
Quaterna * QuaternaBuffer::GetLowestSorted() const
{
	CRAG_VERIFY(* this);
	
	if (_quaterne_sorted_end == _quaterne) 
	{
		// We're clean out of nodes!
		// Unless the node tree is somehow eating itself,
		// this probably means a root nodes is expanding
		// and there are no nodes. 
		return nullptr;
	}
	
	// Ok, lets try the end of the sequence of sorted quaterne...
	Quaterna & reusable_quaterna = _quaterne_sorted_end [- 1];
	return & reusable_quaterna;
}

void QuaternaBuffer::DecrementSorted()
{
	ASSERT(_quaterne_sorted_end > _quaterne);
	CRAG_VERIFY(* this);
	
	-- _quaterne_sorted_end;
	
	CRAG_VERIFY(* this);
}

Quaterna & QuaternaBuffer::Grow()
{
	ASSERT(size() < capacity());
	CRAG_VERIFY(* this);
	
	auto & allocated = * _quaterne_used_end;
	++ _quaterne_used_end;
	
	CRAG_VERIFY(* this);

	return allocated;
}

void QuaternaBuffer::Shrink()
{
	CRAG_VERIFY(* this);

	ASSERT(size() > 0);
	
	-- _quaterne_used_end;
	
	_quaterne_sorted_end = std::min(_quaterne_sorted_end, _quaterne_used_end);
	
	CRAG_VERIFY(* this);
}

bool QuaternaBuffer::empty() const
{
	return _quaterne_used_end == _quaterne;
}

int QuaternaBuffer::size() const
{
	return core::get_index(_quaterne, * _quaterne_used_end);
}

int QuaternaBuffer::capacity() const
{
	return core::get_index(_quaterne, * _quaterne_end);
}

Quaterna & QuaternaBuffer::operator [] (int index)
{
	CRAG_VERIFY_OP(index, <, size());
	return _quaterne[index];
}

Quaterna const & QuaternaBuffer::operator [] (int index) const
{
	return const_cast<QuaternaBuffer &>(* this).operator [] (index);
}

Quaterna & QuaternaBuffer::front()
{
	ASSERT(! empty());
	return * _quaterne;
}

Quaterna const & QuaternaBuffer::front() const
{
	ASSERT(! empty());
	return * _quaterne;
}

Quaterna & QuaternaBuffer::back()
{
	ASSERT(! empty());
	return _quaterne_used_end [-1];
}

Quaterna const & QuaternaBuffer::back() const
{
	ASSERT(! empty());
	return _quaterne_used_end [-1];
}

Quaterna * QuaternaBuffer::begin()
{
	return _quaterne;
}

Quaterna const * QuaternaBuffer::begin() const
{
	return _quaterne;
}

Quaterna * QuaternaBuffer::end()
{
	return _quaterne_used_end;
}

Quaterna const * QuaternaBuffer::end() const
{
	return _quaterne_used_end;
}
