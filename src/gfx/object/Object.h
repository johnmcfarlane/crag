//
//  gfx/object/Object.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Layer.h"

#include "geom/Ray.h"
#include "geom/Matrix33.h"
#include "geom/Matrix44.h"
#include "geom/Transformation.h"


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class LeafNode;
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
		
		typedef double Scalar;
		typedef Vector<Scalar, 3> Vector;
		typedef Matrix<Scalar, 3, 3> Matrix33;
		typedef Matrix<Scalar, 4, 4> Matrix44;
		typedef Transformation<Scalar> Transformation;
		typedef Ray<Scalar, 3> Ray;
		
		enum NodeType
		{
			leaf,
			branch
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Object(Layer::Map::type layers, NodeType node_type);
		virtual ~Object();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		virtual void Init();	// called on arrival in render thread 
		virtual void Deinit();
		
		// returns true iff this object belongs in the given render layer;
		// currently must remain invariant
		bool IsInLayer(Layer::type layer) const;
		bool IsInLayers(Layer::Map::type layers) const;
		
		// typically called by derived class
		void AddToLayer(Layer::type layer);
		
		// scene graph types/variables/functions
		DEFINE_INTRUSIVE_LIST(Object, ChildList);

		NodeType GetNodeType() const;
		
		LeafNode & GetLeafNodeRef();
		LeafNode const & GetLeafNodeRef() const;
		LeafNode * GetLeafNodePtr();
		LeafNode const * GetLeafNodePtr() const;
		
		BranchNode & GetBranchNodeRef();
		BranchNode const & GetBranchNodeRef() const;
		BranchNode * GetBranchNodePtr();
		BranchNode const * GetBranchNodePtr() const;
		
		BranchNode * GetParent();
		BranchNode const * GetParent() const;
		
		void SetParent(BranchNode * parent);
	private:
		BranchNode * _parent;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Layer::Map::type _layers;
		NodeType _node_type;
	};
}
