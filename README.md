Turtlebot3 ball follower application

I. Setup:

a. Hardware:

    https://emanual.robotis.com/docs/en/platform/turtlebot3/overview/ 
    we used the burger

    https://www.raspberrypi.org/products/raspberry-pi-4-model-b/

    A USB camera
    we used: logitech c920

    We replaced the lidar installation of the turtlebot with the USB camera

b. Software

    Raspberry 4 vxWorks SDK
    https://labs.windriver.com/downloads/wrsdk.html#read
    By following the instructions from the associated readme you should have a complete sdcard on top of which we will install the needed libraries to run the demo.

    Ros2 for vxWorks:
    https://github.com/Wind-River/vxworks7-ros2-build
    Following the build instructions using the rasppbery pi 4 SDK should get you a ros2 installation

    Opencv for vxWorks:
    https://github.com/Wind-River/opencv/tree/wrlabs/vxworks_misc
    Following the build instructions should get you a opencv installation for vxworks

    Ros2 application
    following the instruction from  ./robot_control/README.md should get you the ros2 application compiled and ready to test


Optional:
You can download https://github.com/giampaolo/pyftpdlib
and copy the https://github.com/giampaolo/pyftpdlib/tree/master/pyftpdlib directory on the sdcard (sysroot/usr/lib/python3.8/site-packages)
this will give you access to a fast ftp server to copy all the other necesary pieces after boot.

here is a fast script to get you started:

```
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

authorizer = DummyAuthorizer()
authorizer.add_user("user", "12345", "/usr", perm="elradfmwMT")

handler = FTPHandler
handler.authorizer = authorizer

server = FTPServer(("0.0.0.0", 21), handler)
server.serve_forever()
```
II. Install

    Ros2:
    Copy the resulted libraries from the export directory on to the raspberry pi sd-card
    everything in export/lib goes in sysroot/usr/lib/ and everything from export/bin goes in sysroot/usr/bin

    copy the burger.yaml from the repository on the raspberry sd-card (we copied in sysroot/usr/bin)

    OpenCV:
    Copy everything from the install directory (DCMAKE_INSTALL_PREFIX) on the sd-card (sysroot/usr/bin and sysroot/usr/lib), whatever method you choose for copying make sure it follows the symlinks and does not directly copy the symlink (cp -L )

    Imutils:
    Everything from https://github.com/jrosebr1/imutils/tree/master/imutils
    goes in sysroot/usr/lib/python3.8/site-packages

    Paho mqtt:
    Everything from https://github.com/eclipse/paho.mqtt.python/tree/master/src in sysroot/usr/lib/python3.8/site-packages


    Ros2 application:
    Copy the resulted t3_control executable from step I (usually we put it in sysroot/usr/bin)

    Web server application:
    Copy the file webview_image/capture.html in sysroot/usr/bin
    
    Mosquitto 
    copy the contents of /rpi4_sdk/bsps/rpi_4_0_1_1_0/boot/sdcard on your sdcard 


III. Start everything up


on vxworks:
It is useful to start multiple terminals to vxworks using telnet (5 should be enough)

ros2 node
```
-> cmd

# C putenv "LD_LIBRARY_PATH=/sd0a/sysroot/lib;/sd0a/sysroot/usr/lib"
# rtp exec -u 0x80000 /usr/bin/turtlebot3_ros -- -i /usb2ttyS/0 __params:=/usr/bin/burger.yaml

```
mosquitto brocker
```
-> cmd
# rtp exec -u 0x80000 mosquitto.vxe -- -c /mosquitto.conf
```

ros2 app
```
-> cmd
# C putenv "LD_LIBRARY_PATH=/sd0a/sysroot/lib;/sd0a/sysroot/usr/lib"
# rtp exec -u 0x80000 t3_control
```

object tracking application
```
-> cmd
rtp exec -u 0x80000 python3 BallPublisher.py
```

object tracking visualization
```
-> cmd
# cd /ramdisk
# cp /usr/bin/capture.html /ramdisk
# python3 -m http.server 80
```
Open the your browser at http://rasp_ip/capture.html


NOTE!
The object tracking app filters out a object of a given color, it might behave randomly for you at first, we have a filter fine tuning app in
cv_filter_mqtt/
you can start the cv_filter_gui.py on your host with network access to the raspberry pi
edit the line
broker="192.168.0.160"
and match it with the ip of your raspberry py
$ python3 cv_filter_gui.py

this will open a GUI which will allow you to filter the color of the object you want followed, by moving the 6 sliders available (first 3 are more important) you should see the filtered object with white on the second half of the image visualization.

OPTIONAL:
you can run the BallPublisher.py on your host if you want to try it out
change the broker ip to match the one of your raspberry pi and the image display option can be changed to cv2.imshow

```
    cv2.imshow("Result", pano) #linux display
    
    #cv2.imwrite("/ramdisk/capturepy.png", pano) # vxworks html image show
    
    #vxworks  mqtt image publish start
    '''
    payload_frame = pano.tobytes() # vxworks  mqtt image publish..
    print(pano.shape)
    if(payload_frame != ''):
        ret = client3.publish(topic_frame, payload_frame)
    '''
    #vxworks  mqtt image publish stop
```









