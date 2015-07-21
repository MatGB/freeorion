import sys
from random import random
from math import sin, cos, pi

import freeorion as fo


def execute_turn_events():
    print "Executing turn events for turn", fo.current_turn()

    # creating fields
    if random() < max(0.2 * (fo.get_universe_width() / 1000.0), 0.1):
        if random() < 0.3:
            field_type = "FLD_MOLECULAR_CLOUD"
            size = 120.0
        else:
            field_type = "FLD_ION_STORM"
            size = 50.0

        center_x = center_y = fo.get_universe_width() / 2.0
        angle = random() * 2.0 * pi
        x = center_x + ((center_x + 150.0 + (random() * 50)) * sin(angle))
        y = center_y + ((center_x + 150.0 + (random() * 50)) * cos(angle))

        print "...creating new", field_type, "field at:", x, "/", y
        if fo.create_field(field_type, x, y, size) == fo.invalid_object():
            print >> sys.stderr, "Turn events: couldn't create new field"

    return True
