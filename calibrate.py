#!/usr/bin/env python3

import odrive
import odrive.enums
import time

input('Please connect exactly one ODrive with motor on M0 '
      'and then press enter. ')
print('Searching for ODrive...')
time.sleep(1)
print('(Remember, facts don\'t care about your feelings.)')

odrv0 = odrive.find_any()

print('Found ODrive!')
time.sleep(1.5)
print('(There is no such thing as "your truth."\n'
      ' There is the truth and your opinion.)')
time.sleep(1.5)
print('Attempting to calibrate...')

odrv0.axis0.requested_state = odrive.enums.AXIS_STATE_MOTOR_CALIBRATION
time.sleep(6)

print('Finished. Errors:')
print('Axis: 0x{:03x}'.format(odrv0.axis0.error))
print('Motor: 0x{:03x}'.format(odrv0.axis0.motor.error))
print('Controller: 0x{:03x}'.format(odrv0.axis0.controller.error))
print('Estimator: 0x{:03x}'.format(odrv0.axis0.sensorless_estimator.error))
print('(It is not right that children be dunked headfirst\n'
      ' into the vat of garbage we call popular culture.)') 
