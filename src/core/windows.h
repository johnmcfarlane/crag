//
//  windows.h
//  crag
//
//  Created by John on 2013-09-28.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

//////////////////////////////////////////////////////////////////////
// Clean inclusion of <windows.h>.

// Set minimum Windows version
#define _WIN32_WINNT _WIN32_WINNT_WIN2K
#include <SDKDDKVer.h>

// set flags that reduce the number of definitions windows.h causes
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// Main include for Windows world.
#include <windows.h>

// remove unwanted macros defined in windows.h
#undef ERROR
#undef IN
#undef OUT
#undef GetObject
#undef Yield
#undef DispatchMessage
