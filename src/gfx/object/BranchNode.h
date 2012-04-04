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


DECLARE_CLASS_HANDLE(gfx, BranchNode);	// gfx::BranchNodeHandle


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
		virtual void Verify() const override;
#endif
		
		void Init(gfx::Engine const & renderer, Transformation const & transformation);
		
		bool IsEmpty() const;
		
		bool IsChild(Object const & child) const;
		
		friend void AdoptChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child);
		
		List::iterator Begin();
		List::const_iterator Begin() const;
		
		List::iterator End();
		List::const_iterator End() const;
		
		// Model-view transformation
		gfx::Transformation const & Transform(gfx::Engine & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
		Transformation const & GetTransformation() const;
		void SetTransformation(Transformation const & transformation);
		
		Transformation GetModelTransformation() const;
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		List _children;
		Transformation _transformation;
	};
}
