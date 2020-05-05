1. Update SDK

Copy the files in rpi4_sdk to your Raspberry Pi 4 SDK and then update the SD card (you need to copy the files in rpi4_sdk/bsps/rpi_4_0_1_1_0/boot/sdcard/ to the SD card you use to boot the rpi)

2. MQT client app for VxWorks

source wrsdk-vxworks7-raspberrypi4/toolkit/wind_sdk_env.linux
$CC mqtt_cb_client.c -o mqtt_cb_client.vxe -lmosquitto -lssl -lcrypto -ljson

3. Deploy

Copy mqtt_cb_client.vxe to the SD card. Start rpi4 and run the following commands in the vxworks C shell
cmd
cd /sd0a
mosquitto.vxe -c /sd0a/mosquitto.conf &

mqtt_cb_client.vxe


4. Test the vxworks mqtt subscriber app

On your your linux host:

mosquitto_pub -h 192.168.0.188 -t /bb -m '{"action": "right", "value": "101"}'

Note: replace 192.168.0.188 with the IP address of the RPi




