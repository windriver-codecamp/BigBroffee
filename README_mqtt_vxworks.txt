1. copy the files in rpi4_sdk to your Raspberry Pi 4 SDK and then update the SD card (you need to copy the files in rpi4_sdk/bsps/rpi_4_0_1_1_0/boot/sdcard/sysroot/ to sysroot on the SD card you use to boot the rpi)

2. build mqtt client app for VxWorks
source wrsdk-vxworks7-raspberrypi4/toolkit/wind_sdk_env.linux
$CC mqtt_cb_client.c -o mqtt_cb_client.vxe -lmosquitto -lssl -lcrypto -ljson

Copy mqtt_cb_client.vxe to the SD card

3. Deploy

Start rpi4 and run the following commands inthe vxworks C shell
cmd
cd /sd0a
mosquitto.vxe -c /sd0a/mosquitto.conf &

mqtt_cb_client.vxe


#test the vxworks mqt subscriber app
#from your linux host:

#replace 192.168.0.188 with the IP address of the RPi
mosquitto_pub -h 192.168.0.188 -t /bb -m '{"action": "right", "value": "101"}'



