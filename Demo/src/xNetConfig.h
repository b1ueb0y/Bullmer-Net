/*
* @note		����������������
* @author	��Andy.Ro
* @email	��Qwuloo@qq.com
* @date		��2014-06
*/
#ifndef _XNET_CONFIG_H_INCLUDE_
#define _XNET_CONFIG_H_INCLUDE_

#define localhost 127.0.0.1


// ��¼�������ͻ������Ӷ˿ڷ�Χ
#define LOGIN_C2L_PORT			5000
#define LOGIN_C2L_PORT_SCOPE	1000
// ��¼�����������ip��ַ���˿�(��Ϊ�ͻ���)
#define LOGIN_L2M_ADDR			"127.0.0.1"
#define LOGIN_L2M_PORT			MAST_P2M_PORT
#define LOGIN_L2M_PORT_SCOPE	MAST_P2M_PORT_SCOPE

// �û��������ͻ������Ӷ˿ڷ�Χ
#define USER_C2S_PORT			7000
#define USER_C2S_PORT_SCOPE		1
// �û�������ƽ̨��ip��ַ���˿�(��Ϊ�ͻ���)
#define USER_S2P_ADDR			"127.0.0.1"
#define USER_S2P_PORT			PLAT_S2P_PORT
#define USER_S2P_PORT_SCOPE		PLAT_S2P_PORT_SCOPE

// ƽ̨�������û������Ӷ˿ڷ�Χ
#define PLAT_S2P_PORT			8000
#define PLAT_S2P_PORT_SCOPE		1
// ƽ̨�����������ip��ַ���˿�(��Ϊ�ͻ���)
#define PLAT_P2M_ADDR			"127.0.0.1"
#define PLAT_P2M_PORT			MAST_P2M_PORT
#define PLAT_P2M_PORT_SCOPE		MAST_P2M_PORT_SCOPE

// ���������ƽ̨�����Ӷ˿ڷ�Χ
#define MAST_P2M_PORT			9000
#define MAST_P2M_PORT_SCOPE		1

#endif // _XNET_CONFIG_H_INCLUDE_