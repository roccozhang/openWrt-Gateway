
#include "mqtt_client.h"
#include "MQTTPacket.h"

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "thread_mqtt.h"
#include "debug.h"

#define MQTT_PACKET_SIZE  2048


void *mqtt_thread_f(void *parameter)
{
	int msgtype;
	char topic_string[32];
	int ret = 0;	
	int api = 0;
	
	uint8_t buf[MQTT_PACKET_SIZE];
	pthread_detach(pthread_self());
	struct tGatewayInfo *gatewayInfo = (struct tGatewayInfo *)parameter;
	ret = mqtt_connect();
	if(ret == 0)
	{
		DEBUG_LOG("mqtt connect success\n");
	}
	while(1)
	{	
		memset(buf, 0, MQTT_PACKET_SIZE);
		msgtype = MQTTPacket_read(buf, MQTT_PACKET_SIZE,  &mqtt_getdata);

		if (msgtype == PUBLISH) 	/* 判断是否是服务器推送的消息 */
		{
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char *payload_in;

			MQTTString receivedTopic;

			MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic, &payload_in, &payloadlen_in, buf, MQTT_PACKET_SIZE); 	/*序列化mqtt消息*/

			DEBUG_LOG("payload:%d %s  \n", payloadlen_in, payload_in);
			if(payloadlen_in==MQTT_PACKET_SIZE) 
			{
				DEBUG_LOG("mqtt data is to long\n");
				continue;
			}
			
			ret = mqtt_get_api((char *)payload_in, &api);
			if(ret!=0)
			{
				DEBUG_LOG("pares api cmd err ret = %d \r\n", ret);
				continue;
			}
			/* 网关校时，认证，心跳和ota */

			/* 设备管理：添加，删除，编辑 */
			/* 情境管理：添加，删除，编辑 */
			/* 定时管理：添加，删除，编辑 */
			/* 上报api，使用socket */

			/* 控制api，使用串口 */
			
		}
	}
	pthread_exit(0);
}

void mqtt_thread_init(struct tGatewayInfo *gatewayInfo)
{
	int temp; 
	/*创建线程*/
	if((temp = pthread_create(&gatewayInfo->thread.mqtt, NULL, mqtt_thread_f, (void *)gatewayInfo)) != 0)     
		DEBUG_LOG("mqtt thread is err!\n");
	else
		DEBUG_LOG("mqtt thread is ok\n");

}

void mqtt_thread_wait(struct tGatewayInfo *gatewayInfo)
{
	/*等待线程结束*/
	if(gatewayInfo->thread.mqtt !=0)
	{ 
		pthread_join(gatewayInfo->thread.mqtt,NULL);
		DEBUG_LOG("mqtt thread is over/n");
	}
}



