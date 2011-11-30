# main.py
# crag
# The main script file
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.

import crag
import gc
import math
import random
import stackless
import traceback

exec(open("script/observer.py").read())


print('begin main script function')
gc.disable()
stackless.run()


observer_x = 0
observer_y = 9999400
observer_z = 0
max_shapes = 50
shape_drop_period = .5
cleanup_shapes = True


def spawn_shape(shapes):
	if cleanup_shapes:
		if len(shapes) > max_shapes:
			shapes.pop(0)
	else:
		if len(shapes) >= max_shapes:
			return
	
	x = random.random() - .5
	y = observer_y
	z = -4.5 + random.random()
	r = random.choice(['ball','box'])
	if r == 'ball':
		r = math.exp(- random.random() * 2)
		shapes.append(crag.Ball(x, y, z, r))
	elif r == 'box':
		w = math.exp(random.uniform(0, -2))
		l = math.exp(random.uniform(0, -2))
		h = math.exp(random.uniform(0, -2))
		shapes.append(crag.Box(x, y, z, w, l, h))


def main_loop():
	# Set camera position
	crag.set_camera(observer_x, observer_y, observer_z)

	# Create planets
	planet_radius = 10000000
	planet = crag.Planet(0, 0, 0, planet_radius, 3634, 0)
	moon1 = crag.Planet(planet_radius * 1.5, planet_radius * 2.5, planet_radius * 1., 1500000, 10, 250)
	moon2 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 2500000, 13, 0)
	
	# Create sun. 
	sun_orbit_distance = 100000000.
	sun_year = 30000.
	sun = crag.Star(sun_orbit_distance, sun_year)
	
	## Create vehicle
	#v = crag.Vehicle(observer_x, observer_y + 10, observer_z, 1)
	#v.add_rocket(.5, .5, .5, 0, -1, 0, SDL_SCANCODE_SPACE)
	#v.add_rocket(.5, .5, -.5, 0, -1, 0, SDL_SCANCODE_SPACE)
	#v.add_rocket(-.5, .5, .5, 0, -1, 0, SDL_SCANCODE_SPACE)
	#v.add_rocket(-.5, .5, -.5, 0, -1, 0, SDL_SCANCODE_SPACE)
	
	# Create observer (after formations have had time to expand)
	crag.sleep(.5)
	o = observer(observer_x, observer_y, observer_z)
	
	# Main loop
	next_drop = crag.time() + shape_drop_period
	shapes = []
	
	try:
		# Tasklets don't get cleaned up following exceptions
		observer_tasklet = stackless.tasklet(o.run)()
		#crag.attach_bodies(v, o.get_entity())
		
		while stackless.runcount > 1:
			stackless.schedule()
			now = crag.time()
			if now > next_drop:
				spawn_shape(shapes)
				next_drop = now + shape_drop_period
	except Exception as e: 
		print('error in main loop:')
		traceback.print_exc()
		observer_tasklet.kill()


try:
	main_loop()
except Exception as e: 
	print('error in main_loop function:')
	traceback.print_exc()

# Give entities a chance to be destroyed.
print('end main script function')
