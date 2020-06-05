import paho.mqtt.client as paho
import numpy as np
import time
import cv2
import os


broker="localhost"
topic="frame"
port=1883
width = 640
height = 240

def on_publish(client,userdata,result):
  pass

def frame_pub():
  client1= paho.Client("frame_pub")
  client1.on_publish = on_publish
  client1.connect(broker,port,keepalive=60)
  print("Connecting to host",broker)
  cap = cv2.VideoCapture(0)
  cap.set(cv2.CAP_PROP_FPS, 30)
  cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
  cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
  while True:
    payload = ""
    ret, frame1 = cap.read()
    if frame1.shape[2] == 4:
        frame = cv2.cvtColor(frame1, cv2.COLOR_BGRA2BGR)
    if frame is None:
      break
    payload = frame.tobytes()
    if(payload != ''):
        ret = client1.publish(topic,payload)

  cap.release()

frame_pub()

