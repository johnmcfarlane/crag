//
//  Object.h
//  crag
//
//  Created by john on 2014-07-05.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Formation.h"

#include "ipc/ObjectBase.h"

namespace form
{
	class Engine;
	
	class Object final : public ipc::ObjectBase<Object, Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		using super = ipc::ObjectBase<Object, Engine>;
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Object(Engine & engine, Formation formation);
		~Object();
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Formation _formation;
	};
}
