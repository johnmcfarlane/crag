import gc
import crag
import time

def main():
	print 'begin main script function'

	# Create planets
	planet_radius = 10000000
	planet = crag.Planet(0,0,0,planet_radius,8,0)
	moon1 = crag.Planet(planet_radius * 1.5, planet_radius * 2.5, planet_radius * 1., 1500000, 10, 250)
	moon2 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 2500000, 13, 0)

	# Create sun. 
	sun_orbit_distance = 100000000.
	sun_year = 30000.
	sun = crag.Star(sun_orbit_distance, sun_year)

	# Create observer
	observer = crag.Observer(0, 10000235, 0)

	# Main loop
	while crag.done() == 0:
		time.sleep(0.01)

	print 'end main script function'

gc.disable()
main()
