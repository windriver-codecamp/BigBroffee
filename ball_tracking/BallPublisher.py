from __future__ import print_function
from imutils.video import WebcamVideoStream
from imutils.video import VideoStream
from imutils.video import FPS
import paho.mqtt.client as paho
from datetime import datetime
from collections import deque
import numpy as np
import argparse
import imutils
import time
import cv2
import os

broker="localhost" 
topic="test"
port=1883
horizontal_threshold = 10
radius_threshold = 5
horizontal_limit = 500
vertical_limit = 400
normal_radius_threshold = 40
close_radius_threshold = 55

def on_publish(client,userdata,result):
  print("Published data is : ")
  pass

def ball_pub():
  client1= paho.Client("control1") #create client object
  client1.on_publish = on_publish #assign function to callback
  client1.connect(broker,port,keepalive=60) #establishing connection
  ap = argparse.ArgumentParser()
  ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
  ap.add_argument("-n", "--num-frames", type=int, default=100,
	help="# of frames to loop over for FPS test")
  ap.add_argument("-d", "--display", type=int, default=-1,
	help="Whether or not frames should be displayed")
  args = vars(ap.parse_args())
  print("[INFO] sampling frames from webcam...")
  blueLower = (87, 78, 121)
  blueUpper = (255, 255, 255)
  # greenLower = (29, 86, 6)
  # greenUpper = (64, 255, 255)
  pts = deque(maxlen=args["buffer"])
  vs = VideoStream(src=0).start()
  fps = FPS().start()
  time.sleep(2)
  radius1 = 0
  while True:
    frame = vs.read()
    if frame is None:
        break
    frame = imutils.resize(frame, width = horizontal_limit)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, blueLower, blueUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None
    if args["display"] > 0:
      cv2.imshow("Frame", frame)
      key = cv2.waitKey(1) & 0xFF
    fps.update()
    if len(cnts) > 0:
      payload = ''
      c = max(cnts, key=cv2.contourArea)
      ((x, y), radius) = cv2.minEnclosingCircle(c)
      M = cv2.moments(c)
      center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
      if radius > 10:
        cv2.circle(frame, (int(x), int(y)), int(radius),(0, 255, 255), 2)
        cv2.circle(frame, center, 5, (0, 0, 255), -1)
        if radius <= close_radius_threshold and radius > normal_radius_threshold:
          cv2.putText(frame, "Normal.", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius > close_radius_threshold:
          cv2.putText(frame, "Close", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius < normal_radius_threshold:
          cv2.putText(frame, "Far", (350, 300), cv2.FONT_HERSHEY_PLAIN,fontScale=1.5, color=(0, 100, 100), thickness=2)
      if(radius-radius1 > radius_threshold):
          payload += '{"action": "forward", "value": "' + str(radius-radius1)+'"}'
      elif(radius-radius1 < -radius_threshold):
          payload += '{"action": "backward", "value": "' + str(-(radius-radius1)) +'"}'
      radius1=radius
    pts.appendleft(center)

    for i in range(1, len(pts)):
      if pts[i - 1] is None or pts[i] is None:
              continue
      elif pts[i][0] > 0 and pts[i][0] < horizontal_limit/2 and pts[i][1] > 0 and pts[i][1] < vertical_limit/2:
              w = "Left-up"
      elif pts[i][0] > 0 and pts[i][0] < horizontal_limit/2 and pts[i][1] > vertical_limit/2 and pts[i][1] < vertical_limit:
          w = "Left-down"
      elif pts[i][0] > horizontal_limit/2 and pts[i][0] < horizontal_limit and pts[i][1] > 0 and pts[i][1] < vertical_limit/2:
          w = "Right-up"
      elif pts[i][0] > horizontal_limit/2 and pts[i][0] < horizontal_limit and pts[i][1] > vertical_limit/2 and pts[i][1] < vertical_limit:
          w = "Right-down"
      elif pts[i][0] == horizontal_limit/2 and pts[i][1] == vertical_limit/2:
          w = "Center!"
      if(pts[i][0]-pts[i-1][0] > horizontal_threshold):
            payload += '{"action": "left", "value": "' + str(pts[i][0]-pts[i-1][0]) + '"}'
      elif(pts[i][0]-pts[i-1][0] < -horizontal_threshold):
            payload += '{"action": "right", "value": "' + str(-(pts[i][0]-pts[i-1][0])) + '"}'
      thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
      cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
      cv2.putText(frame, w, (350, 350), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
      if(payload != ''):
        ret = client1.publish(topic,payload) #topic name is test
        print(payload)
        print("Please check data on your Subscriber Code \n")
    cv2.imshow("Frame", frame)  
    key = cv2.waitKey(1) & 0xFF
    fps.stop()
    # print("[INFO] elasped time: {:.2f}".format(fps.elapsed()))
    # print("[INFO] approx. FPS: {:.2f}".format(fps.fps()))
    if key == ord("q"):
        break
    
  vs.release()
  cv2.destroyAllWindows()  

ball_pub()
