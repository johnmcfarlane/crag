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

exec(open("script/observer.py").read())


print('begin main script function')
gc.disable()
stackless.run()

def main_loop():
	# Set camera position
	crag.set_camera(0, 10000580, 0)

	# Create planets
	planet_radius = 10000000
	planet = crag.Planet(0, 0, 0, planet_radius, 3634, 0)
	crag.sleep(.1)

	moon1 = crag.Planet(planet_radius * 1.5, planet_radius * 2.5, planet_radius * 1., 1500000, 10, 250)
	moon2 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 2500000, 13, 0)
	
	# Create sun. 
	sun_orbit_distance = 100000000.
	sun_year = 30000.
	sun = crag.Star(sun_orbit_distance, sun_year)
	
	# Create observer (after formations have had time to expand)
	crag.sleep(.5)
	o = observer()
	observer_tasklet = stackless.tasklet(o.run)()
	
	# Main loop
	drop_period = .5
	next_drop = crag.time() + drop_period
	shapes = []
	ball = True
	while stackless.runcount > 1:
		now = crag.time()
		if now > next_drop and len(shapes) < 100:
			x = random.random() - .5
			y = 10000580
			z = -4.5 + random.random()
			if ball:
				r = math.exp(- random.random() * 2)
				shapes.append(crag.Ball(x, y, z, r))
			else:
				w = math.exp(- random.random() * 2)
				l = math.exp(- random.random() * 2)
				h = math.exp(- random.random() * 2)
				shapes.append(crag.Box(x, y, z, w, l, h))
			ball = not ball
			if len(shapes) > 50:
				shapes.pop(0)
			next_drop = now + drop_period
		stackless.schedule()
  
main_loop()

# Give entities a chance to be destroyed.
crag.sleep(0)
print('end main script function')
