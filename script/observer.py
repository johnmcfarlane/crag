# main.py
# crag
# All things related to the object housing the camera.
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.


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
 

class observer_task:
	def __init__(self):
		self.__observer = crag.Observer(0, 10000235, 0)
		self.__is_done = False

	def done(self):
		return self.__is_done
	
	# returns True if an event was handled
	def handle_event(self):
		event = crag.get_event()
		
		if event[0] == "exit":
			self.__is_done = True
		elif event[0] == "key":
			self.handle_keyboard(event[1], event[2] == 1)
		elif event[0] == "mousemove":
			self.handle_mousemove(float(event[1]), float(event[2]))
		else:
			return False
		return True

	# returns True if an event was handled
	def handle_keyboard(self, scancode, is_down):
		if is_down:
			if scancode == SDL_SCANCODE_ESCAPE:
				self.__is_done = True
			elif scancode == SDL_SCANCODE_0:
				self.__observer.set_speed(0)
			elif scancode >= SDL_SCANCODE_1 and scancode <= SDL_SCANCODE_9:
				self.__observer.set_speed(scancode + 1 - SDL_SCANCODE_1)

	# returns True if an event was handled
	def handle_mousemove(self, x_delta, y_delta):
		self.sensitivity = 0.1
		x_rotation = - y_delta * self.sensitivity
		y_rotation = - x_delta * self.sensitivity
		self.__observer.add_rotation(x_rotation, 0.0, y_rotation)

def run_observer():
	print('begin run_observer')

	o = observer_task()
	while not o.done():
		if not o.handle_event():
			stackless.schedule()

	print('end run_observer')
