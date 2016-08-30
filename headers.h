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
#include "lwip/tcp.h"
#include <ipv4/lwip/ip_addr.h>
#include <espressif/user_interface.h>
#include <esp/timer.h>
#include <esp/uart.h>
#include <espressif/esp_common.h>
#include <sdk_internal.h>
#include <lwip/sys.h>
#include <sdk_internal.h>
#include <espressif/esp_common.h>
#include <timers.h>
#include <esp/gpio.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <stdbool.h>
#include <portmacro.h>
#include <esplibs/libmain.h>
#include <ipv4/lwip/ip_nat.h>


#define ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
#define GPFFS_GPIO(p) (((p)==0||(p)==2||(p)==4||(p)==5)?0:((p)==16)?1:3)

//GPIO 16 Control Registers
#define GP16O  ESP8266_REG(0x768)
#define GP16E  ESP8266_REG(0x774)
#define GP16I  ESP8266_REG(0x78C)

//GPIO 16 PIN Control Register
#define GP16C  ESP8266_REG(0x790)
#define GPC16  GP16C

//GPIO 16 PIN Function Register
#define GP16F  ESP8266_REG(0x7A0)
#define GPF16  GP16F

#define GP16FFS0 0 //Function Select bit 0
#define GP16FFS1 1 //Function Select bit 1
#define GP16FPD  3 //Pulldown
#define GP16FSPD 5 //Sleep Pulldown
#define GP16FFS2 6 //Function Select bit 2
#define GP16FFS(f) (((f) & 0x03) | (((f) & 0x04) << 4))

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


#endif
