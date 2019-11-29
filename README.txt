Description
-----------

A short program that uses an Arduino to monitor an MPU-6050 MEMS gyroscope/accelerometer.
The results are averaged over a short period and sent over a serial connection along
with the measurement time.

Possible improvements
---------------------
* Use JSON instead of my own string formatting, just to be more in line with convention.
* Double check that the settings byte is set correctly for maximum precision.
