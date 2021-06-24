import cv2 as cv
import paho.mqtt.client as paho


broker="192.168.0.160"
topic="filter"
port=1883

alpha_slider_max = 255
title_window = 'Linear Blend'
trackbar_name1 = 'Min1 x %d' % alpha_slider_max
trackbar_name2 = 'Min2 x %d' % alpha_slider_max
trackbar_name3 = 'Min3 x %d' % alpha_slider_max

trackbar_name4 = 'Max1 x %d' % alpha_slider_max
trackbar_name5 = 'Max2 x %d' % alpha_slider_max
trackbar_name6 = 'Max3 x %d' % alpha_slider_max

def on_publish(client,userdata,result):
  print("Published data is : ")
  print(userdata)
  pass

def on_trackbar(val):
    payload = ''
    alpha = val / alpha_slider_max
    beta = ( 1.0 - alpha )
    #print (alpha, beta)
    v1 = cv.getTrackbarPos(trackbar_name1, title_window)
    v2 = cv.getTrackbarPos(trackbar_name2, title_window)
    v3 = cv.getTrackbarPos(trackbar_name3, title_window)

    v4 = cv.getTrackbarPos(trackbar_name4, title_window)
    v5 = cv.getTrackbarPos(trackbar_name5, title_window)
    v6 = cv.getTrackbarPos(trackbar_name6, title_window)
    payload  =  "%d, %d, %d, %d, %d, %d" %  (v1, v2, v3, v4, v5, v6)
    if(payload != ''):
        ret = client1.publish(topic,payload) #topic name is test
        print(payload)

client1= paho.Client("cv_filter") #create client object
client1.on_publish = on_publish #assign function to callback
client1.connect(broker,port,keepalive=6000) #establishing connection


cv.namedWindow(title_window)

cv.createTrackbar(trackbar_name1, title_window , 87, alpha_slider_max, on_trackbar)
cv.createTrackbar(trackbar_name2, title_window , 78, alpha_slider_max, on_trackbar)
cv.createTrackbar(trackbar_name3, title_window , 121, alpha_slider_max, on_trackbar)

cv.createTrackbar(trackbar_name4, title_window , 255, alpha_slider_max, on_trackbar)
cv.createTrackbar(trackbar_name5, title_window , 255, alpha_slider_max, on_trackbar)
cv.createTrackbar(trackbar_name6, title_window , 255, alpha_slider_max, on_trackbar)

# Show some stuff
on_trackbar(0)
# Wait until user press some key
cv.waitKey()
