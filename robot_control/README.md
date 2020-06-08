<b style="font-size:18px;">For building the project:</b>

1.Clone the repo: https://github.com/Wind-River/vxworks7-ros2-build.git<br>
and follow the instructions here: https://github.com/Wind-River/vxworks7-ros2-build<br>
2.Download the QEMU IA sdk from windriver labs. Add files here according to the readme from: http://bitbucket.wrs.com/users/mdragusu/repos/ball_follower_robot/browse/mqtt_vxworks/qemu_ia_sdk <br>
<div style="text-decoration: line-through;">3.Follow the instructions I through IV here: http://bitbucket.wrs.com/users/mdragusu/repos/ros2t3/browse<br>
4.Overwrite files inside ros2t3 with the ones from: http://bitbucket.wrs.com/users/mdragusu/repos/ball_follower_robot/browse/robot_control<br>
5.inside a terminal, change directory to <i>vxworks7-ros2-build</i><br>
</div>
Work in progress...

6.sudo docker run -ti -v /path/to/sdk/wrsdk-vxworks7-qemu:/wrsdk -v $PWD:/work vxros2build:1.0 <br>
<div>$ source /work/build/ros2/ros2_ws/install/setup.bash
$ cd obj_avoid_t3/ <br>
$ cmake . <br>
$ make <br>
</div>

<b style="font-size:18px;">For testing the project:</b>

move the compiled files into the /export/root folder (or wherever the vxworks fs is mounted)

1st terminal:<br>
qemu-system-x86_64 -m 512M  -kernel $WIND_SDK_TOOLKIT/../bsps/itl_generic_2_0_2_1/boot/vxWorks -net nic  -net user,hostfwd=tcp::1534-:1534,hostfwd=tcp:127.0.0.1:8023-:23,hostfwd=tcp::11883-:1883 \<br>
-display none -serial mon:stdio \<br>
-append "bootline:fs(0,0)host:vxWorks h=10.0.2.2 e=10.0.2.15 u=username pw=password o=gei0" \<br>
-usb -device usb-ehci,id=ehci  \<br>
-device usb-storage,drive=fat32 -drive file=fat:rw:./export/root,id=fat32,format=raw,if=none<br>
(if you do not modify the kernel path to the location where your sdk is located, it might be necessary to source wrsdk-vxworks7-qemu/toolkit/wind_sdk_env.linux)<br>

cmd<br>
set env LD_LIBRARY_PATH="/bd0a/lib"<br>
cd /bd0a<br>
mosquitto.vxe -c mosquitto.conf &<br>

2nd terminal:<br>
telnet 127.0.0.1 8023<br>
cmd<br>
C putenv "LD_LIBRARY_PATH=/bd0a/usr/lib"<br>
rtp exec -u 0x30000 obj_avoid_t3 0.04 0.4 10 1 0.1 0.035 0.15<br>
(here the main application will run and receive mqtt messages (from the 4th terminal) and publish them, along with publishing manual controls.)<br>

3rd terminal:<br>
telnet 127.0.0.1 8023<br>
cmd<br>
C putenv "LD_LIBRARY_PATH=/bd0a/usr/lib"<br>
rtp exec -u 0x30000 test_subscriber<br>
(here test_subscriber will run and receive messages from the ROS publisher, and print the current linear and radial values on the screen.)<br>

4th terminal:<br>
mosquitto_pub -h 127.0.0.1 -p 11883 -t /bb -m '{"action": "left", "value": "1"}'<br>
(here we publish a test mqtt message via mosquitto.)
