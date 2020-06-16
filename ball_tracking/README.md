# ball_tracking

# Overview

Detect the presence of a colored ball using OpenCV and Python3. Track the ball as it moves around in the video frames, drawing its previous positions as it moves in BallPublisher.py and transmit the coordonates using MQTT messages to BallSubscriber.py. To assure the accuracy of detecting the ball use the ../cv_filter_mqtt/cv_filter_gui.py to adjust the colour parameters of the ball.

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
$ cd ball_tracking/
$ python3 ../cv_filter_mqtt/cv_filter_gui.py
$ python3 BallPublisher.py
$ python3 BallSubscriber.py
```