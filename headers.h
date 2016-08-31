#ifndef HEADERS_H
#define HEADERS_H

#include <string.h>
#include <espressif/esp_common.h>
#include <FreeRTOS.h>
#include <task.h>
#include <dhcpserver.h>
#include <stdlib.h>
#include <esp/types.h>
#include <lwip/api.h>
#include <lwip/tcp.h>
#include <ipv4/lwip/ip_addr.h>
#include <espressif/user_interface.h>
#include <esp/timer.h>
#include <esp/uart.h>
#include <espressif/esp_common.h>
#include <sdk_internal.h>
#include <lwip/sys.h>
#include <espressif/esp_common.h>
#include <timers.h>
#include <esp/gpio.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <stdbool.h>
#include <portmacro.h>
#include <esplibs/libmain.h>
#include <ipv4/lwip/ip_nat.h>
#include <esp/spi.h>
#include <adafruit_sharpmemory_display.h>
#include <flirlipton.h>

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_RATE_MS)

#define GPIO_PB1     	(0)
#define GPIO_LED	 	(1)
#define GPIO_SPEAKER 	(2)
#define GPIO_SPI_MISO	(12)
#define GPIO_SPI_MOSI 	(13)
#define GPIO_SPI_CLK 	(14)
#define GPIO_Camera_CS  (15)
#define GPIO_DISPLAY_CS (16)


extern struct station_info* sdk_wifi_softap_get_station_info();
extern bool sdk_wifi_softap_free_station_info();
extern int VersionNumber;
extern xQueueHandle ButtonQueue;
extern sys_mutex_t SPI_Mutex;


#endif
