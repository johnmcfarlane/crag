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
	crag.sleep(.2)
	o = observer()
	observer_tasklet = stackless.tasklet(o.run)()
	
	# Main loop
	next_drop = time.time() + 1
	balls = []
	while stackless.runcount > 1:
		now = time.time()
		if now > next_drop:
			next_drop = now + 1
			balls.append(crag.Ball(random.random() - .5, 10000580, -4.5 + random.random(), math.exp(- random.random() * 2)))
			if len(balls) > 25:
				balls.pop(0)
		stackless.schedule()

main_loop()

# Give entities a chance to be destroyed.
crag.sleep(0)
print('end main script function')
