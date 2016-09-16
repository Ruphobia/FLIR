#include "camera.h"
#include "userinterface.h"

FLIRBuffer CameraBuffer;
static uint16_t SocketImageBuffer[80*60];


sys_mutex_t CameraBufferMutex;
int ColdBaby = 0;

static struct tcp_pcb *ControlTCPServer = NULL;
static int ControlConnections = 0;

static void CaptureCameraImage()
{
	FLIR_Lipton_CaptureImage(CameraBuffer);

	sys_mutex_lock(&CameraBufferMutex);

	int i = 0;
	for(int y = 0; y < 60; y++)
		for(int x = 1; x < 41; x++)
		{
			SocketImageBuffer[i] = (uint16_t)((CameraBuffer[x][y] & 0xFFFF0000) >> 16);
			i++;
			SocketImageBuffer[i] = (uint16_t)(CameraBuffer[x][y] & 0x0000FFFF);
			i++;
		}
	sys_mutex_unlock(&CameraBufferMutex);
}

static void Control_Close_conn(struct tcp_pcb *pcb)
{
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_close(pcb);

	ControlConnections--;
	if (ControlConnections < 0)
		ControlConnections = 0;
}

//socket receive call back
static err_t Control_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	if (err == ERR_OK && p != NULL)
	{
		//tell TCP stack that we have received this packet
		tcp_recved(pcb, p->tot_len);

		//if the packet is not empty do something with it
		if (p->tot_len > 0)
		{
			//get an image
			if ( !(strstr(p->payload, "680900") == NULL) )
			{
				printf("Capturing Image\n");
				CaptureCameraImage();
				printf("Sending Image\n");
				tcp_write(ControlTCPServer,SocketImageBuffer,9600, 0);
				printf("Sent\n");
				tcp_output(ControlTCPServer);
			}
		}
		pbuf_free(p);
	} else
	{
		pbuf_free(p);
	}

	if (err == ERR_OK && p == NULL)
	{
		Control_Close_conn(pcb);
	}
	return ERR_OK;
}

//tcp socket accept call back
err_t Control_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	//what does this do and why do
	//we need it?
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	tcp_setprio(pcb, TCP_PRIO_MIN);//set priority
	tcp_recv(pcb, Control_recv);//set rx call back
	tcp_err(pcb, NULL); //Don't care about error here
	tcp_poll(pcb, NULL, 4); //No polling here

	ControlConnections++;//increment socket connection count

	//set our server socket equal to the
	//Incoming socket for transmit purposes.
	//Note: we expect that we will only talk to
	//one socket at a time, so we are only
	//concerned with the latest connection.
	//this could lead to some issues if multiple
	//devices connect to the softAP then
	//connect to this port.  It might be
	//better to maintain a list of connect
	//sockets then send all TX to all.
	//only testing will confirm if this is OK...
	ControlTCPServer = pcb;

	return ERR_OK;
}


void MeasureBabyTask(void *pvParameters)
{
	while(1)
	{
		vTaskDelayMs(5000);

		CaptureCameraImage();

		sys_mutex_lock(&CameraBufferMutex);

		double ave = 0;
		int j = 0;
		int f = 0;

		for(j = 0; j < 4800; j++)
		{
			if (SocketImageBuffer[j] >= 8200)
			{
				ave += SocketImageBuffer[j];
				f++;
			}
		}

		if (f > 0)
		{
			printf("%d\n",f);
			ave /= f;
			printf("%d\n", (int) ave);

			if ((f >60) && (f < 800))
			{//we have baby
				printf("Baby!!!\n");
				if (ave < 8340)
					//we have a cold baby
					Alarm_ON();
				else
					Alarm_OFF();


			}
			else
				Alarm_OFF();

		}
		else
			Alarm_OFF();

		sys_mutex_unlock(&CameraBufferMutex);

		if (AlarmIsOn)
			printf("Alarm On\n");
		else
			printf("Alarm Off\n");

	}
}
//crate and socket listen, loop forever
//accepting incoming sockets
void ControlCreateSocketTask(void *pvParameters)
{
	struct tcp_pcb *ptel_pcb;
	ptel_pcb = tcp_new();//create new TCP socket

	tcp_bind(ptel_pcb, IP_ADDR_ANY, 35000);//bind new socket to port

	while (1)
	{
		ptel_pcb = tcp_listen(ptel_pcb);//listen to socket
		tcp_accept(ptel_pcb, Control_accept);//set accept call back

		vTaskDelayMs(1000);
		taskYIELD();
	}

}
