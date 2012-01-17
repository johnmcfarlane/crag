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
		
		// Model-view transformation
		gfx::Transformation const & Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
		Transformation const & GetTransformation() const;
		void SetTransformation(Transformation const & transformation);
		
		Transformation GetModelTransformation() const;
		
		void Update(UpdateParams const & params, Renderer & renderer);
		
	private:
		BranchNode * CastListObject() override;
		BranchNode const * CastListObject() const override;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		ChildList _children;
		Transformation _transformation;
	};
}
