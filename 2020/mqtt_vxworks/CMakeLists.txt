set (_sources mqtt_cb_client.c)
set (_app "mqtt_cb_client.vxe")
add_executable (${_app} ${_sources})
target_link_libraries (${_app} mosquitto ssl crypto json)


