//**********READ ME**********************************
// MAKE sure you set "TCP_SND_BUF" in opt.h to
// at least 10, or you will get buffer overflows on
// socket transmits.
//
//***************************************************

#include "headers.h"
#include "userinterface.h"
#include "camera.h"
#include "user_config.h"



sys_mutex_t SPI_Mutex;

//********************************************************
//***********DO NOT REMOVE UNTIL BUG IS FIXED*************
//********************************************************
//This is here to fix the "bug" where you get
//9999999999 max connection messages.
//TODO: wait for update to ESP-OPEN-RTOS to
//resolve this issue.
//reference google groups:
//https://groups.google.com/forum/#!topic/esp-open-rtos/4BlYYrYRfH8
void sdk_hostap_handle_timer(void *cnx_node)
{
}

//entry point into user space,
//this is "like" void main()
void user_init()
{
	//disable system debug printing to
	//UART0
	system_set_os_print(0);

	//set CPU clock frequency to 160Mhz
	//sdk_os_update_cpu_frequency(160);

	//create a mutex for the SPI port
	//so the display and the camera
	//don't step on each other
	sys_mutex_new(&SPI_Mutex);
	sys_mutex_new(&CameraBufferMutex);

	//yet another bug workaround, we need to make sure the station is disconnected before
	//we allow the user to initiate a AP scan or we be crashing bang, boom, ding, bing, bong...
	//TODO:  Fix this with updated SDK
	sdk_wifi_station_disconnect();

	uart_set_baud(0, 115200);//ELM327 runs at 9600bps

	//get the mac address of the access point
	//so we can add that to the SSID
	uint8_t mac[6];
	sdk_wifi_get_macaddr(SOFTAP_IF, (uint8_t*)&mac[0]);

	//setup soft access point
	struct ip_info info;
	static struct sdk_softap_config cfg;
	sdk_wifi_softap_get_config(&cfg);
	//create SSID with last 2 of MAC appended to SSID
	sprintf((char*)&cfg.ssid[0], "WIFI_%02x%02x", mac[4], mac[5]);
	cfg.ssid_len = strlen((char*)cfg.ssid);
	cfg.authmode = AUTH_WPA_WPA2_PSK;
	cfg.max_connection = 5;

	memcpy(cfg.password, password, passwdlen);
	sdk_wifi_softap_set_config(&cfg);
	sdk_wifi_set_opmode(SOFTAP_MODE);//We are both an access point and station

	IP4_ADDR(&info.ip, 192, 168, 0, 10);
	IP4_ADDR(&info.gw, 192, 168, 0, 10);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	sdk_wifi_set_ip_info(SOFTAP_IF, &info);

	ip_addr_t first_client_ip;
	IP4_ADDR(&first_client_ip, 192, 168, 0, 100);
	dhcpserver_start(&first_client_ip, 4);

	struct sdk_station_config config =
	{
			.ssid = "",
			.password = "",
	};

	//yet another bug workaround, we need to make sure the station is disconnected before
	//we allow the user to initiate a AP scan or we be crashing bang, boom, ding, bing, bong...
	//TODO:  Fix this with updated SDK
	sdk_wifi_station_set_config(&config);
	sdk_wifi_station_disconnect();

	gpio_enable(GPIO_PB1, GPIO_INPUT);//push button 1
	gpio_enable(GPIO_LED, GPIO_OUTPUT);//LED (leave disabled during debugging, shares UART0 TXD
	gpio_write(GPIO_LED,0);

	gpio_enable(GPIO_SPEAKER, GPIO_OUTPUT);//Speaker
	gpio_write(GPIO_SPEAKER,0);

	gpio_enable(GPIO_Camera_CS,GPIO_OUTPUT);

	i2c_init(5,4);

	Adafruit_Sharpmemory_Display_Init(GPIO_DISPLAY_CS,&SPI_Mutex);
	FLIR_Lipton_Init(GPIO_Camera_CS, &SPI_Mutex);

	xTaskCreate(ControlCreateSocketTask, (signed char *)"ControlTCP", 512, NULL, 2, NULL);//Control socket thread
	xTaskCreate(MeasureBabyTask, (signed char *)"MeasureBabyTask", 512, NULL, 2, NULL);//Control socket thread

}



