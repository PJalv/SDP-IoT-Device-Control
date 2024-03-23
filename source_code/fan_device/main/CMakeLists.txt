# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS  "main.c" "../../utils/wifi.c" "../../utils/mqtt.c" "storage_nvs.c" "audio.c" "fan.c" "init.c" "tasks.c" )
set(COMPONENT_ADD_INCLUDEDIRS "../../utils")

register_component()
spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)