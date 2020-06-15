from __future__ import print_function
from imutils.video import WebcamVideoStream
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
from datetime import datetime



broker="192.168.0.160"
topic="/bb"
topic2="filter"
topic_frame="frame"
port=1883
horizontal_threshold = 10
radius_threshold = 5
horizontal_limit = 320
vertical_limit = 240
normal_radius_threshold = 40
close_radius_threshold = 55
mes = "150, 150, 121, 255, 255, 255"



def on_message(client, userdata, message):
    global mes
    print("Received data is :")
    print(str(message.payload.decode("utf-8")) )
    mes =str(message.payload.decode("utf-8"))

def on_publish(client,userdata,result):
  #print("Published data is : ")
  pass

def on_publish_frame(client,userdata,result):
  #print("Published data is : ")
  pass

def ball_pub():
  client1= paho.Client("control1")
  client1.on_publish = on_publish
  client1.connect(broker,port,keepalive=6000)
  client2= paho.Client("control2")
  client2.on_message = on_message
  print("Connecting to host",broker)
  client2.connect(broker, port, keepalive=6000)
  print("Subscribing begins here")
  client2.subscribe(topic2)

  #client3= paho.Client("frame_pub")
  #client3.on_publish = on_publish_frame
  #client3.connect(broker,port,keepalive=60)


  ap = argparse.ArgumentParser()
  ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
  ap.add_argument("-n", "--num-frames", type=int, default=100,
	help="# of frames to loop over for FPS test")
  ap.add_argument("-d", "--display", type=int, default=-1,
	help="Whether or not frames should be displayed")
  args = vars(ap.parse_args())
  pts = deque(maxlen=args["buffer"])
  left_cnt = 0
  right_cnt = 0
  center_treshold = 50
  vs = VideoStream(src=0).start()
  vs.stream.set(cv2.CAP_PROP_FPS, 5)

  time.sleep(2)
  radius1 = 0
  t1 = datetime.now()
  t2 = t1
  publish_interval = 10

  while True:
    frame1 = vs.read()
    #client2.loop_start()
    #client1.loop_start()
    lis = (mes.split(','))
    liss = [int(i.strip()) for i in lis]
    blueLower = (liss[0], liss[1], liss[2])
    blueUpper = (liss[3], liss[4], liss[5])
    if frame1 is None:
        break
    if frame1.shape[2] == 4:
        frame = cv2.cvtColor(frame1, cv2.COLOR_BGRA2BGR)
    else:
        frame = frame1
    frame = imutils.resize(frame, width = horizontal_limit)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, blueLower, blueUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None
    mask = imutils.resize(mask, width=horizontal_limit)
    mask = cv2.cvtColor(mask, cv2.COLOR_GRAY2RGB)
    imgs = [frame, mask]
    foo_height, foo_width = imgs[0].shape[:2]
    bar_height, bar_width = imgs[1].shape[:2]
    pano = np.zeros((foo_height, foo_width+bar_width, 3), np.uint8)
    #pano = np.hstack((imgs[0], imgs[1]))
    #cv2.imshow("Result", pano)
    key = cv2.waitKey(1) & 0xFF
    if len(cnts) > 0:
      payload = ''
      c = max(cnts, key=cv2.contourArea)
      ((x, y), radius) = cv2.minEnclosingCircle(c)
      M = cv2.moments(c)
      center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
      if radius > 10:
        cv2.circle(frame, (int(x), int(y)), int(radius),(0, 255, 255), 2)
        cv2.circle(frame, center, 5, (0, 0, 255), -1)
        '''
        if radius <= close_radius_threshold and radius > normal_radius_threshold:
          cv2.putText(frame, "Normal.", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius > close_radius_threshold:
          cv2.putText(frame, "Close", (350, 300), cv2.FONT_HERSHEY_PLAIN, fontScale=1.5, color=(0, 100, 100), thickness=2)
        elif radius < normal_radius_threshold:
          cv2.putText(frame, "Far", (350, 300), cv2.FONT_HERSHEY_PLAIN,fontScale=1.5, color=(0, 100, 100), thickness=2)
          '''
      #for forward/backward movement
      '''
      if(radius-radius1 > radius_threshold):
          payload += '{"action": "forward", "value": "' + str(radius-radius1)+'"}'
      elif(radius-radius1 < -radius_threshold):
          payload += '{"action": "backward", "value": "' + str(-(radius-radius1)) +'"}'
      radius1=radius
      '''
    pts.appendleft(center)

    for i in range(1, len(pts)):
      if pts[i - 1] is None or pts[i] is None:
              continue
      '''
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
      '''
      if(pts[i][0] < horizontal_limit/2 and horizontal_limit/2 - pts[i][0] >= abs(center_treshold)):
        left_cnt += 1
      elif(pts[i][0] > horizontal_limit/2 and pts[i][0] - horizontal_limit/2 >= abs(center_treshold)):
        right_cnt += 1
      elif(horizontal_limit/2 - pts[i][0] >= abs(center_treshold)):
        left_cnt = 0
        right_cnt = 0

      '''
      thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
      cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
      '''
      t2 = datetime.now()
      if ((t2 - t1).microseconds > 200000):
        if(left_cnt > right_cnt):
          payload = '{"action": "left", "value": "' + '1' + '"}'
        elif(right_cnt > left_cnt):
          payload = '{"action": "right", "value": "' + '1' + '"}'
        elif (right_cnt == 0 and left_cnt == 0):
          payload = '{"action": "right", "value": "' + '0' + '"}'
        
        if(payload != ''):
          ret = client1.publish(topic, payload, qos=0, retain=False)
          print(payload)
        payload = ''
        left_cnt = 0
        right_cnt = 0
        t1 = datetime.now()

    pano = np.hstack((imgs[0], imgs[1]))
    cv2.imshow("Result", pano) #linux display

    #cv2.imwrite("/ramdisk/capturepy.png", pano) # vxworks html image show
    
    #vxworks  mqtt image publish start
    '''
    payload_frame = pano.tobytes() # vxworks  mqtt image publish  
    print(pano.shape)
    if(payload_frame != ''):
        ret = client3.publish(topic_frame, payload_frame)
    '''
    #vxworks  mqtt image publish stop

    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break

  #vs.release()
  #cv2.destroyAllWindows()

ball_pub()
