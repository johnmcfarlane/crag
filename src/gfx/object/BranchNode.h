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


namespace gfx { DECLARE_CLASS_HANDLE(BranchNode); }	// gfx::BranchNodeHandle


namespace gfx
{
	// function declarations
	void AdoptChild(Object & child, BranchNode & parent);
	void OrphanChild(Object & child, BranchNode & parent);
	void OrphanChild(Object & child);

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

		DECLARE_ALLOCATOR(BranchNode);

		BranchNode(Init const & init, Transformation const & transformation);
		~BranchNode();
		
#if defined(VERIFY)
		virtual void Verify() const override;
#endif
		
		bool IsEmpty() const;
		
		bool IsChild(Object const & child) const;
		
		friend void AdoptChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child);
		
		List::iterator Begin();
		List::const_iterator Begin() const;
		
		List::iterator End();
		List::const_iterator End() const;
		
		Object & Front();
		Object const & Front() const;
		
		Object & Back();
		Object const & Back() const;
		
		// Model-view transformation
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
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
