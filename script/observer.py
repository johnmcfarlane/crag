# main.py
# crag
# All things related to the object housing the camera.
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.


# from SDL_keycode.h
SDLK_ESCAPE = 27
SDLK_0 = 48
SDLK_1 = 49
SDLK_9 = 57


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
	def handle_keyboard(self, keysym, is_down):
		if is_down:
			if keysym == SDLK_ESCAPE:
				self.__is_done = True
			elif keysym == SDLK_0:
				self.__observer.set_speed(0)
			elif keysym >= SDLK_1 and keysym <= SDLK_9:
				self.__observer.set_speed(keysym + 1 - SDLK_1)

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
