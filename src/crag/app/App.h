/*
 *  Input.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/Vector2.h"

#include "AppSdl.h"

#include "glpp/gl.h"
// TODO: use ptime
//#include "boost/date_time/posix_time/posix_time.hpp"

namespace app
{
	// input
	bool IsKeyDown(KeyCode key_code);
	bool IsButtonDown(MouseButton mouse_button);

	Vector2i GetMousePosition();
	void SetMousePosition(Vector2i const & position);

	// window
	Vector2i GetWindowSize();

	bool GetEvent(Event & event);
	bool HasFocus();

	// GL
	bool InitGl();

	// time
	TimeType GetTime();
	void Sleep(TimeType t);

	double TimeTypeToSeconds(TimeType t);
	TimeType SecondsToTimeType(float s);
}
