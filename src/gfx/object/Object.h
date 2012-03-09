//
//  gfx/object/Object.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/defs.h"


DECLARE_CLASS_HANDLE(gfx, Object)	// gfx::ObjectHandle


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class LeafNode;
	class Pov;
	class Renderer;
	class Scene;

	// Base class for drawable things.
	// Note that these have intrusive list/tree entries (esp. in BranchNode)
	// meaning they effectively double as nodes in a hierachical scene graph.
	class Object : public smp::ObjectBase<Object, Renderer>
	{
		OBJECT_NO_COPY(Object);
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef smp::ObjectBase<Object, Renderer> super;
		
		enum NodeType
		{
			leaf,
			branch
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Object(NodeType node_type);
		virtual ~Object();
		
#if defined(VERIFY)
		virtual void Verify() const override;
#endif
		
		virtual void Deinit(Scene & scene);
		
		virtual Transformation const & Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const;
		
		// scene graph types/variables/functions
		NodeType GetNodeType() const;
		
		LeafNode & CastLeafNodeRef();
		LeafNode const & CastLeafNodeRef() const;
		LeafNode * CastLeafNodePtr();
		LeafNode const * CastLeafNodePtr() const;
		
		BranchNode & CastBranchNodeRef();
		BranchNode const & CastBranchNodeRef() const;
		BranchNode * CastBranchNodePtr();
		BranchNode const * CastBranchNodePtr() const;
		
		BranchNode * GetParent();
		BranchNode const * GetParent() const;
		
		friend void AdoptChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child, BranchNode & parent);
		friend void OrphanChild(Object & child);
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		BranchNode * _parent;
		NodeType const _node_type;
	};
}
