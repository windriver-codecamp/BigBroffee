1. Update SDK

Copiaza fisierele din qemu_ia_sdk in SDK-ul tau. Va fi nevoie sa copiezi fisierele cu numele care incepe cu libjson din qemu_ia_sdk/toolkit/include/usr/lib/common si in ./export/root/lib (directorul produs de buildul de ROS2)

2. Subscriber MQTT - va face parte din control.cpp 

source wrsdk-vxworks7-qemu/toolkit/wind_sdk_env.linux
$CC mqtt_cb_client.c -o mqtt_cb_client.vxe -lmosquitto -lssl -lcrypto -ljson

Sau varianta cu cmake:
mkdir ball_follower_robot/mqtt_vxworks/build
cd ball_follower_robot/mqtt_vxworks/build
cmake ../
make

3. Deployment

Copiaza fisierul mosquitto.conf in directorul de la paragraful 1.

Copiaza toate binarele pe care vrei sa le rulezi in directorul de la sectiunea 1. Este acelasi director care apare si in comanda urmatoare qemu. Schimba calea /home/dan/projects/cto/sdk/src/wrsdk-release-script/wrsdk-vxworks7-qemu-wrcc640/sdk/wrsdk-vxworks7-qemu-wrcc640/workspace cu o cale catre un director ales de tine. De exemplu la ROS2 va fi calea catre ./export/root.

qemu-system-x86_64 -m 512M  -kernel bsps/itl_generic_2_0_2_1/boot/vxWorks -net nic \
-net user,hostfwd=tcp::1534-:1534,hostfwd=tcp:127.0.0.1:8023-:23,hostfwd=tcp::11883-:1883 \
-display none -serial mon:stdio \
-append "bootline:fs(0,0)host:vxWorks h=10.0.2.2 e=10.0.2.15 u=target pw=vxTarget o=gei0" \
-usb -device usb-ehci,id=ehci  \
-device usb-storage,drive=fat32 -drive file=fat:rw:/home/dan/projects/cto/sdk/src/wrsdk-release-script/wrsdk-vxworks7-qemu-wrcc640/sdk/wrsdk-vxworks7-qemu-wrcc640/workspace,id=fat32,format=raw,if=none


#vxworks shell in QEMU
cmd
set env LD_LIBRARY_PATH="/bd0a/lib"
cd /bd0a
mosquitto.vxe -c mosquitto.conf &

/bd0a/lib in guestul VxWorks va fi mapat pe directorul de pe host: /home/dan/projects/cto/sdk/src/wrsdk-release-script/wrsdk-vxworks7-qemu-wrcc640/sdk/wrsdk-vxworks7-qemu-wrcc640/workspace/lib

In acest director trebuie sa fie prezente librariile dinamice de care are nevoie mqtt_cb_client.vxe:
libmosquitto.so.1, libssl.so.1, libjson.so.1 si libc.so.1.

#pe host, sesiune telnet catre qemu:
telnet 127.0.0.1 8023

#shell VxWorks in telnet
cmd
cd /bd0a
mqtt_cb_client.vxe

Pentru a valida conectivitatea, poti folosi:

#pe host
mosquitto_pub -h 127.0.0.1 -p 11883 -t /bb -m '{"action": "right", "value": "101"}'


