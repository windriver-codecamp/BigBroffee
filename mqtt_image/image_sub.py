import paho.mqtt.client as mqtt
import numpy as np
import json
import PIL
import cv2
from matplotlib import pyplot as plt

broker="localhost"  #host name
topic="frame" #topic name
port=1883
#cv2.namedWindow('image', cv2.WINDOW_NORMAL)
array = np.zeros((448,800,3), dtype=np.uint8)
#img = np.zeros((448,800,3))

def on_message(client, userdata, message):
  global array
  global img
  print("Received data is :")
  data = json.loads(message.payload)
  array = np.array(data,dtype=np.uint8)

client= mqtt.Client("frame_sub") #create client object
client.on_message=on_message
print("Connecting to host",broker)
client.connect(broker, port, keepalive=60)#connection establishment with broker
print("Subscribing begins here")
client.subscribe(topic)#subscribe topic test

while 1:
  client.loop_start() #contineously checking for message
  cv2.imshow('image', array)
  cv2.waitKey(1)

cv2.destroyAllWindows()
