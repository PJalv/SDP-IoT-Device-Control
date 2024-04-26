idf_component_register(SRCS "main.c" "../../utils/wifi.c" "../../utils/mqtt.c" "led_config.c" "storage_nvs.c"
                       INCLUDE_DIRS ".")
                       set(COMPONENT_ADD_INCLUDEDIRS "../../utils")
spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)