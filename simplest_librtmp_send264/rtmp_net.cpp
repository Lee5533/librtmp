#include "rtmp_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librtmp/rtmp.h"   
#include "librtmp/rtmp_sys.h"   
#include "librtmp/amf.h"  

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

#ifdef WIN32     
#include <windows.h>  
#pragma comment(lib,"WS2_32.lib")   
#pragma comment(lib,"winmm.lib")  
#endif 

RTMP* m_pRtmp;

/**
* ��ʼ��winsock
*
* @�ɹ��򷵻�1 , ʧ���򷵻���Ӧ�������
*/
int InitSockets()
{
#ifdef WIN32     
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else     
	return TRUE;
#endif     
}

/**
* ��ʼ�������ӵ�������
*
* @param url �������϶�Ӧwebapp�ĵ�ַ
*
* @�ɹ��򷵻�1 , ʧ���򷵻�0
*/

int Net_Init(const char* url)
{
	InitSockets();

	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
	//����URL
	if (RTMP_SetupURL(m_pRtmp, (char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		return -1;
	}
	//���ÿ�д,��������,�����������������ǰʹ��,������Ч
	RTMP_EnableWrite(m_pRtmp);
	//���ӷ�����
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		return -1;
	}

	//������
	if (RTMP_ConnectStream(m_pRtmp, 0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return -1;
	}

	return 0;
}

void CleanSockets()
{
	// �ͷ�winsock  @�ɹ��򷵻�0 , ʧ���򷵻���Ӧ�������
#ifdef WIN32     
	WSACleanup();
#endif  
}

/**
* �Ͽ����ӣ��ͷ���ص���Դ��
*
*/
void Net_Close()
{
	if (m_pRtmp)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}

	CleanSockets();
}

/**
* ����RTMP���ݰ�
*
* @param nPacketType ��������
* @param data �洢��������
* @param size ���ݴ�С
* @param nTimestamp ��ǰ����ʱ���
*
* @�ɹ��򷵻� 1 , ʧ���򷵻�һ��С��0����
*/

int SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp)
{
	int nRet = 0;
	RTMPPacket* packet = NULL;

	//������ڴ�ͳ�ʼ��,lenΪ���峤��
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
	memset(packet, 0, RTMP_HEAD_SIZE);

	//�����ڴ�
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body, data, size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType; //�˴�Ϊ����������һ������Ƶ,һ������Ƶ
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	//packet->m_nInfoField2 = 0;
	packet->m_nChannel = 0x04; 


	//if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	//packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

	packet->m_nTimeStamp = nTimestamp;
	//����

	if (RTMP_IsConnected(m_pRtmp))
	{
		nRet = RTMP_SendPacket(m_pRtmp, packet, FALSE); //TRUEΪ�Ž����Ͷ���,FALSE�ǲ��Ž����Ͷ���,ֱ�ӷ���
	}
	//�ͷ��ڴ�
	free(packet);

	return nRet;
}
