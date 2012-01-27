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


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class LeafNode;
	class Pov;
	class Renderer;
	class Scene;

	class ObjectBase
	{
		DEFINE_INTRUSIVE_LIST(ObjectBase, ChildList);
	};
	
	// Base class for drawable things.
	// Note that these have intrusive list/tree entries (esp. in BranchNode)
	// meaning they effectively double as nodes in a hierachical scene graph.
	class Object : public ObjectBase
	{
		OBJECT_NO_COPY(Object);
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
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
		virtual void Verify() const;
#endif
		
		virtual bool Init(Renderer & renderer);	// called on arrival in render thread 
		virtual void Deinit(Scene & scene);
		
		virtual Transformation const & Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const;
		
		Uid GetUid() const;
		
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
		
		void SetParent(BranchNode * parent);

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		BranchNode * _parent;
		Uid const _uid;
		NodeType const _node_type;
	};
}
