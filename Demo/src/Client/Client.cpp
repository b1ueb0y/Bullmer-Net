// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "../xSockClient/xSockClient.h"
#include "../xConfig/xNetConfig.h"
#include "../xUtils/xUtils.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char buf[][200]=
	{
		"1.��ã�����~~~~~~~~~",
		"2.hello,world !",
		"3.�㵽��������أ��Ҷ����� ��",
		"4.лл���������ң������������ˣ�Ҫ�ĵİ���",
		"5.һ��Ҫ�ǵ�֪ͨ�Ұ�������Ȼ���ٺ�~",
		"6.������",
		"7.�ǵ���!!!",
		"8.�ķ�����!!!",
		"9.���Ž�ͳһ!!!",
		"10.�����ݱ��Ȼ��ͷ!!!",
		"11.�о��������ط���������Ϳѻ!!!",
	};
	srand((u32_t)time(NULL));

	xSockClient* sockClient[100];
	int CONN_NUM = 1;
	for (int i=0; i<CONN_NUM; ++i)
	{
		if ( NO_ERROR == xClientSocketPool::shareObject()->connect(str2ipaddr("127.0.0.1"), USER_C2S_PORT/*+i%50*/,sockClient[i]) )
		{
			printf("success...\n");
// 			int idx = rand()%(sizeof(buf)/sizeof(buf[0]));
// 			if( NO_ERROR != sockClient[i]->send(buf[idx],strlen(buf[idx])) )
// 			{
// 				//break;
// 			}
		}
		else
		{
			printf("failed...\n");
		}
		
	}

	int ii =0;
	while( 1 )
	{
// 		if ( NO_ERROR != sockClient[0]->random_regst_test() )
// 		{
// 		//	getchar();
// 		//	break;
// 		}
		
		int idx = rand()%(sizeof(buf)/sizeof(buf[0]));
		if( NO_ERROR != sockClient[rand()%CONN_NUM]->send(buf[idx],strlen(buf[idx])) )
		{
			printf("�ɹ����͸�����%d\n",++ii);
			//break;
		}
		Sleep(10);
	}
	getchar();
	return 0;
}