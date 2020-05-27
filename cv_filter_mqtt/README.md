# cv_filter_mqtt

# ￼Overview

To assure the accuracy of detecting the ball from ../ball_tracking/ use cv_filter_gui.py to adjust the colour parameters of the ball.

# Prerequisite
## Dependencies
```
$ sudo apt-get install python-opencv
$ pip3 install numpy
$ pip3 install --upgrade imutils
$ pip3 install paho-mqtt
```
# Run
```
$ cd cv_filter_mqtt/
$ python3 cv_filter_gui.py
$ python3 ../ball_tracking/BallPublisher.py
$ python3 ../ball_tracking/BallSubscriber.py
```