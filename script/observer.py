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
 

def run_observer():
	print 'begin run_observer'
	observer = crag.Observer(0, 10000235, 0)
	planet_radius = 25000000
	# moon3 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 15000000, 10, 250)

	while 1:
		event = crag.get_event()
		
		if event[0] == "exit":
			break
		elif event[0] == "keydown":
			if event[1] == SDL_SCANCODE_ESCAPE and event[2] == 1:
				break
			elif event[1] == SDL_SCANCODE_X:
				if event[2] == 0:
					pass
					# moon3 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 15000000, 10, 250)
				else:
					pass
					# moon3 = None
		elif event[0] == "mousemove":
			sensitivity = 0.1
			observer.add_rotation(float(- event[2]) * sensitivity, 0.0, float(- event[1]) * sensitivity)
		else:
			stackless.schedule()

	print 'end run_observer'
