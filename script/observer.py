# main.py
# crag
# All things related to the object housing the camera.
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.

import time

exec(open("script/utility.py").read())


# from SDL_keycode.h
SDLK_ESCAPE = 27
SDLK_0 = 48
SDLK_1 = 49
SDLK_9 = 57
SDLK_c = 99


class observer:
	def __init__(self):
		self.__observer = crag.Observer(0, 10000580, 0)
		self.__is_done = False
		self.__collision = True
	
	def run(self):
		while True:
			event = get_event()
			if self.handle_event(event):
				return

	# returns True if an event was handled
	def handle_event(self, event):
		if event[0] == "exit":
			return True
		elif event[0] == "key":
			return self.handle_keyboard(event[1], event[2] == 1)
		elif event[0] == "mousemove":
			return self.handle_mousemove(float(event[1]), float(event[2]))
		return False

	# returns True if an event was handled
	def handle_keyboard(self, keysym, is_down):
		if is_down:
			if keysym == SDLK_ESCAPE:
				return True
			elif keysym == SDLK_0:
				self.__observer.set_speed(10)
			elif keysym >= SDLK_1 and keysym <= SDLK_9:
				self.__observer.set_speed(keysym + 1 - SDLK_1)
			elif keysym == SDLK_c:
				self.__collision = not self.__collision
				self.__observer.set_collidable(self.__collision)
		return False

	# returns True if an event was handled
	def handle_mousemove(self, x_delta, y_delta):
		self.sensitivity = 0.1
		z_rotation = - x_delta * self.sensitivity
		x_rotation = - y_delta * self.sensitivity
		self.__observer.add_rotation(x_rotation, 0.0, z_rotation)
		return False
