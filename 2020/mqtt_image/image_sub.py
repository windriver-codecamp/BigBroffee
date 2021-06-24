import paho.mqtt.client as mqtt
import numpy as np
import cv2

broker="192.168.0.160"  #host name
topic="frame" #topic name
port=1883

width = 640
height = 180
cv2.namedWindow('image', cv2.WINDOW_NORMAL)
array = np.zeros((height, width,3), dtype=np.uint8)


def on_message(client, userdata, message):
  global array
  global img
  data = message.payload
  #print(data)
  data = np.frombuffer(data, dtype=np.uint8)
  array = data.reshape(height, width, 3)

client= mqtt.Client("frame_sub") #create client object
client.on_message=on_message
print("Connecting to host",broker)
client.connect(broker, port, keepalive=60)#connection establishment with broker
print("Subscribing begins here")
client.subscribe(topic)#subscribe topic test
client.loop_start() #contineously checking for message

while 1:
  cv2.imshow('image', array)
  cv2.waitKey(1)

cv2.destroyAllWindows()
