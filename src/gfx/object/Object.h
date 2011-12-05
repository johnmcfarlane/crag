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
#include "gfx/Layer.h"


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class LeafNode;
	class Pov;
	class Scene;
	
	// Base class for drawable things.
	// Note that these have intrusive list/tree entries (esp. in BranchNode)
	// meaning they effectively double as nodes in a hierachical scene graph.
	class Object
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
		
		Object(NodeType node_type, Layer::Map::type layers = Layer::Map::none);
		virtual ~Object();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		virtual void Init(Scene const & scene);	// called on arrival in render thread 
		virtual void Deinit();
		
		Uid GetUid() const;
		
		// returns true iff this object belongs in the given render layer;
		// currently must remain invariant
		bool IsInLayer(Layer::type layer) const;
		bool IsInLayers(Layer::Map::type layers) const;
		Layer::Map::type GetLayers() const;
		
		// typically called by derived class
		void AddToLayers(Layer::Map::type layers);
		
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
		DEFINE_INTRUSIVE_LIST(Object, ChildList);
		
		Uid const _uid;
		NodeType const _node_type;
		Layer::Map::type _layers;
	};
}
