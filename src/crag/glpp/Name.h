/*
 *  Name.h
 *  Crag
 *
 *  Created by John on 12/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/debug.h"

#include "glpp.h"


namespace gl
{
	
	// Specializations
	template<GLenum TARGET> GLuint Init();
	template<GLenum TARGET> void Deinit(GLuint id);
	template<GLenum TARGET> void Bind(GLuint id);
	template<GLenum TARGET> GLuint GetBindingEnum();


	template<GLenum TARGET> class Name
	{
	public:
		enum {
			TARGET_ENUM = TARGET
		};
		
		Name()
		: id(0)
		{
		}
		
		~Name()
		{
			Deinit();
		}
		
		// Initialization.
		bool IsInitialized() const
		{
			return id != 0;
		}
		
		void Init()
		{
			if (id != 0) {
				gl::Deinit<TARGET>(id);
			}
			id = gl::Init<TARGET>();
		}
		
		void Deinit()
		{
			if (id != 0) {
				gl::Deinit<TARGET>(id);
				id = 0;
			}
		}
		
		// Binding
		bool IsBound() const
		{
			return gl::GetBound(GetBindingEnum<TARGET>()) == id;
		}
		
		template<typename GL_NAME> friend void Bind(GL_NAME * name);
		
	#if defined(DUMP)
		friend std::ostream & operator << (std::ostream &out, Name const & n)
		{
			out << "id:" << n.id;
			return out;
		}
	#endif

	protected:
		GLuint id;
	};


	template<typename GL_NAME> void Bind(GL_NAME * name)
	{
		if (name != nullptr) {
			assert(name->id != 0);
			gl::Bind<GL_NAME::TARGET_ENUM>(name->id);
			assert(name->IsBound());
		}
		else {
			gl::Bind<GL_NAME::TARGET_ENUM>(0);
		}
		
	}

}
