//
//  BranchNode.h
//  crag
//
//  Created by John McFarlane on 2011/11/21.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"

#include "geom/Transformation.h"


namespace gfx
{
	// An object which contains sub-objects
	class BranchNode : public Object
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef Object super;
		
	public:
		struct UpdateParams
		{
			Transformation transformation;
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		BranchNode();
		~BranchNode();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		bool IsEmpty() const;
		
		void AddChild(Object & child);
		void RemoveChild(Object & child);
		
		ChildList::iterator Begin();
		ChildList::const_iterator Begin() const;
		
		ChildList::iterator End();
		ChildList::const_iterator End() const;
		
		Transformation const & GetTransformation() const;
		void SetTransformation(Transformation const & transformation);
		void GetAccumulatedTransformation(Transformation & transformation) const;
		
		void Update(UpdateParams const & params);
		
	private:
		BranchNode * CastListObject() override;
		BranchNode const * CastListObject() const override;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Object::ChildList _children;
		Transformation _transformation;
	};
}
