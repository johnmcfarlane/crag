//
//  applet/defs.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/function_ref.h"

namespace applet
{
	// forward-declare
	class AppletInterface;

	DECLARE_CLASS_HANDLE(Applet);	// applet::AppletHandle

	// condition on which to wake from a WaitFor call
	typedef core::function_ref<bool()> Condition;
	typedef std::function<void (AppletInterface &)> LaunchFunction;
}
