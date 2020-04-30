import paho.mqtt.client as paho
import datetime
import time
import sys

broker="localhost"  #host name
topic="test" #topic name
        
def on_message(client, userdata, message):
  print("Received data is :")  
  print(str(message.payload.decode("utf-8")) ) #printing Received message
  print("")
    
client= paho.Client("user") #create client object 
client.on_message=on_message
   
print("Connecting to host",broker)
client.connect(broker)#connection establishment with broker
print("Subscribing begins here")    
client.subscribe(topic)#subscribe topic test

while 1:
    client.loop_start() #contineously checking for message 
