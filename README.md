# servo-calibration
Arduino sketch to assist in the calibration of continuous full rotation servos

This version of the sketch is based around using a simple photo-resistor and LED as a light
transmitter and detector configured as an optical interrupter.  This is used,
along with a piece of dark card stack attached to the wheel mounted on the
servo to get timing information to calculate the rotation speed of the servo
for various settings.  This is a standalone sketch.  The servo is controlled
separately.  Really this can be used to determine the (revolution) speed of any
rotating object that can have a marker attached, and where the needed sensor
can be positioned close enough.
