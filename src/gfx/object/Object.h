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

#include "geom/Transformation.h"

namespace gfx { DECLARE_CLASS_HANDLE(Object); }	// gfx::ObjectHandle


namespace gfx
{
	// forward-declarations
	class LeafNode;
	class Pov;
	class Engine;
	class Scene;

	// function declarations
	bool IsChild(Object const & child, Object const & parent);
	void AdoptChild(Object & child, Object & parent);
	void OrphanChild(Object & child, Object & parent);
	void OrphanChild(Object & child);

	// Base class for drawable things.
	// meaning they effectively double as nodes in a hierachical scene graph.
	class Object : public ipc::Object<Object, Engine>
	{
	protected:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef ipc::Object<Object, Engine> super;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Object(Init const & init, Transformation const & local_transformation);
	public:
		virtual ~Object();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Object);
		
		// scene graph types/variables/functions
		virtual LeafNode & CastLeafNodeRef();
		virtual LeafNode const & CastLeafNodeRef() const;
		virtual LeafNode * CastLeafNodePtr();
		virtual LeafNode const * CastLeafNodePtr() const;
		
		//////////////////////////////////////////////////////////////////////////////
		// tree structure

	private:
		Object * _parent;
		DEFINE_INTRUSIVE_LIST(Object, List);
		List _children;

	public:
		friend bool IsChild(Object const & child, Object const & parent);
		friend void AdoptChild(Object & child, Object & parent);
		friend void OrphanChild(Object & child, Object & parent);
		friend void OrphanChild(Object & child);
		
		Object * GetParent();
		Object const * GetParent() const;
		
		List & GetChildren();
		List const & GetChildren() const;

		//////////////////////////////////////////////////////////////////////////////
		// transformation

		Transformation const & GetLocalTransformation() const;
		void SetLocalTransformation(Transformation const & local_transformation);
		
		Transformation GetModelTransformation() const;
		
	private:
		Transformation _local_transformation;
	};
}
