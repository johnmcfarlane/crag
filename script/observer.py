# main.py
# crag
# All things related to the object housing the camera.
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.

import time

exec(open("script/utility.py").read())


# from SDL_scancode.h
SDL_SCANCODE_A = 4
SDL_SCANCODE_B = 5
SDL_SCANCODE_C = 6
SDL_SCANCODE_D = 7
SDL_SCANCODE_E = 8
SDL_SCANCODE_F = 9
SDL_SCANCODE_G = 10
SDL_SCANCODE_H = 11
SDL_SCANCODE_I = 12
SDL_SCANCODE_J = 13
SDL_SCANCODE_K = 14
SDL_SCANCODE_L = 15
SDL_SCANCODE_M = 16
SDL_SCANCODE_N = 17
SDL_SCANCODE_O = 18
SDL_SCANCODE_P = 19
SDL_SCANCODE_Q = 20
SDL_SCANCODE_R = 21
SDL_SCANCODE_S = 22
SDL_SCANCODE_T = 23
SDL_SCANCODE_U = 24
SDL_SCANCODE_V = 25
SDL_SCANCODE_W = 26
SDL_SCANCODE_X = 27
SDL_SCANCODE_Y = 28
SDL_SCANCODE_Z = 29

SDL_SCANCODE_1 = 30
SDL_SCANCODE_2 = 31
SDL_SCANCODE_3 = 32
SDL_SCANCODE_4 = 33
SDL_SCANCODE_5 = 34
SDL_SCANCODE_6 = 35
SDL_SCANCODE_7 = 36
SDL_SCANCODE_8 = 37
SDL_SCANCODE_9 = 38
SDL_SCANCODE_0 = 39

SDL_SCANCODE_RETURN = 40
SDL_SCANCODE_ESCAPE = 41
SDL_SCANCODE_BACKSPACE = 42
SDL_SCANCODE_TAB = 43
SDL_SCANCODE_SPACE = 44


class observer:
	def __init__(self, x, y, z):
		self.__observer = crag.Observer(x, y, z)
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
			if keysym == SDL_SCANCODE_ESCAPE:
				return True
			elif keysym == SDL_SCANCODE_0:
				self.__observer.set_speed(10)
			elif keysym >= SDL_SCANCODE_1 and keysym <= SDL_SCANCODE_9:
				self.__observer.set_speed(keysym + 1 - SDL_SCANCODE_1)
			elif keysym == SDL_SCANCODE_C:
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
