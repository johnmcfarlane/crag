# utility.py
# crag
# Functions which block the process.
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.

def get_event():
	while True:
		event = crag.get_event()
		if event != None:
			return event
		else:
			stackless.schedule()
