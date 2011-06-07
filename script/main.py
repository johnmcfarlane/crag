# main.py
# crag
# The main script file
#
# Created by John on 10/3/10.
# Copyright 2010-2011 John McFarlane. All rights reserved.
# This program is distributed under the terms of the GNU General Public License.

import gc
import crag
import stackless
#import time

exec(open("script/observer.py").read())


def main():
    print('begin main script function')
    gc.disable()
    stackless.run()

    # Create planets
    planet_radius = 10000000
    planet = crag.Planet(0, 0, 0, planet_radius, 8, 0)
    moon1 = crag.Planet(planet_radius * 1.5, planet_radius * 2.5, planet_radius * 1., 1500000, 10, 250)
    moon2 = crag.Planet(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1., 2500000, 13, 0)

    # Create sun. 
    sun_orbit_distance = 100000000.
    sun_year = 30000.
    sun = crag.Star(sun_orbit_distance, sun_year)

    # Create observer
    observer_tasklet = stackless.tasklet(run_observer)()

    # Main loop
    while stackless.runcount > 1:
        # Kind-of pointless right now...
        stackless.schedule()

    print('end main script function')


main()
