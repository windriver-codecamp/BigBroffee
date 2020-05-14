import paho.mqtt.client as paho
import numpy as np
import time
import cv2
import os
import json

broker="localhost"
topic="frame"
port=1883


def on_publish(client,userdata,result):
  pass

def frame_pub():
  client1= paho.Client("frame_pub")
  client1.on_publish = on_publish
  client1.connect(broker,port,keepalive=60)
  print("Connecting to host",broker)
  cap = cv2.VideoCapture(2)
  cap.set(cv2.CAP_PROP_FPS, 30)
  while True:
    payload = ""
    ret, frame = cap.read()
    if frame is None:
      break
    frame_list = frame.tolist()
    payload = json.dumps(frame_list)
    if(payload != ''):
        ret = client1.publish(topic,payload)

  cap.release()

frame_pub()

