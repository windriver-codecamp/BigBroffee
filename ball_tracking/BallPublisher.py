from imutils.video import VideoStream
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
topic="test";
port=1883 #MQTT data listening port
# ACCESS_TOKEN='M7OFDCmemyKoi461BJ4j' #not manditory

def on_publish(client,userdata,result):
  print("Published data is : ")
  pass

def ball_pub():
  client1= paho.Client("control1") #create client object
  client1.on_publish = on_publish #assign function to callback
  # client1.username_pw_set(ACCESS_TOKEN) #access token from thingsboard device
  client1.connect(broker,port,keepalive=60) #establishing connection
  ap = argparse.ArgumentParser()
  ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
  args = vars(ap.parse_args())
  blueLower = (87, 78, 121)
  blueUpper = (255, 255, 255)
  # greenLower = (29, 86, 6)
  # greenUpper = (64, 255, 255)
  pts = deque(maxlen=args["buffer"])
  vs = VideoStream(src=0).start()
  time.sleep(2)
  radius1 = 0
  while True:
    frame = vs.read()
    if frame is None:
        break
    frame = imutils.resize(frame, width=500)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, blueLower, blueUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    if len(cnts) > 0:
      payload = ''
      c = max(cnts, key=cv2.contourArea)
      ((x, y), radius) = cv2.minEnclosingCircle(c)
      M = cv2.moments(c)
      center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
      if radius > 10:
        cv2.circle(frame, (int(x), int(y)), int(radius),(0, 255, 255), 2)
        cv2.circle(frame, center, 5, (0, 0, 255), -1)
        if radius <= 55 and radius > 40:
          cv2.putText(frame, "Normal.", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius > 55:
          cv2.putText(frame, "Close", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius < 40:
          cv2.putText(frame, "Far", (350, 300), cv2.FONT_HERSHEY_PLAIN,fontScale=1.5, color=(0, 100, 100), thickness=2)
      if(radius-radius1 > 5):
          payload += '{go fw, ' + str(radius-radius1)+'}'
      elif(radius-radius1 < -5):
          payload += '{go bw, ' + str(-(radius-radius1)) +'}'
      radius1=radius
    pts.appendleft(center)

    for i in range(1, len(pts)):
      if pts[i - 1] is None or pts[i] is None:
              continue
      elif pts[i][0] > 0 and pts[i][0] < 250 and pts[i][1] > 0 and pts[i][1] < 200:
              w = "Left-up"
      elif pts[i][0] > 0 and pts[i][0] < 250 and pts[i][1] > 200 and pts[i][1] < 400:
          w = "Left-down"
      elif pts[i][0] > 250 and pts[i][0] < 500 and pts[i][1] > 0 and pts[i][1] < 200:
          w = "Right-up"
      elif pts[i][0] > 250 and pts[i][0] < 500 and pts[i][1] > 200 and pts[i][1] < 400:
          w = "Right-down"
      elif pts[i][0] == 250 and pts[i][1] == 200:
          w = "Center!"
      if(pts[i][0]-pts[i-1][0] > 10):
            payload +='{go left, ' + str(pts[i][0]-pts[i-1][0]) + '}'
      elif(pts[i][0]-pts[i-1][0] < -10):
            payload +='{go right, ' + str(-(pts[i][0]-pts[i-1][0])) + '}'
      if(pts[i][1]-pts[i-1][1] > 10):
            payload +='{go up, ' + str(pts[i][1]-pts[i-1][1]) + '}'
      elif(pts[i][1]-pts[i-1][1] < -10) :
            payload +='{go down, ' + str(-(pts[i][1]-pts[i-1][1])) + '}'
      thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
      cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
      cv2.putText(frame, w, (350, 350), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
      if(payload != ''):
        ret = client1.publish(topic,payload) #topic name is test
        print(payload)
        print("Please check data on your Subscriber Code \n")
    cv2.imshow("Frame", frame)  
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
  vs.release()
  cv2.destroyAllWindows()  

ball_pub()
