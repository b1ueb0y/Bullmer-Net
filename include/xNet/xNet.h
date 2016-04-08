/*
* @note		������ṹ�嶨��
* @author	��Andy.Ro
* @email	��Qwuloo@qq.com
* @date		��2014-06
*/
#ifndef _XNET_H_INCLUDE_
#define _XNET_H_INCLUDE_

#include "../Interface/xTypes.h"
#include "../xSocket/xSocket.h"
#include "../Interface/xPacket.h"

/*
*
*	TCP ������:
*		1. �����׽���(socket);
*
*		2. ���׽��ֵ����ض˿ں�IP��ַ(bind);
*
*		3. �����׽���Ϊ����ģʽ(listen),׼�����ܿͻ���������;
*
*		4. ������������(accept),�����½������ӵ��׽���;
*
*		5. ���ѽ������ӵ��׽��ֺͿͻ��˽���ͨ��(send/recv);
*
*		6. �ر��׽���;
*
*	TCP �ͻ���:
*		1. �����׽���(socket);
*
*		2. �������������������(connect);
*
*		3. �����������ͨ��(send/recv);
*
*		4. �ر��׽���;
*/

/*
*
*	UDP ������:
*		1. �����׽���(socket);
*
*		2. ���׽��ֵ����ض˿ں�IP��ַ(bind);
*
*		3. �ȴ�����(recvfrom)����������(sendto);
*
*		4. �ر��׽���;
*
*
*	UDP �ͻ���:
*		1. �����׽���(socket);
*
*		2. �����������(sendto)����������(recvfrom);
*
*		3. �ر��׽���;
*
*/

//------------------------------------------------------------------------
// io_t
//------------------------------------------------------------------------
typedef struct _io_t
{
#ifdef WIN32
	WSAOVERLAPPED	ol;
#endif

	// clientfd
	SOCKET			fd;

	char*			buf;
	u32_t			len;

	// ��Ϊ�����
#define OP_ACCEPT	0
	// ��Ϊ�ͻ���
#define OP_CONNECT	1

#define OP_READ		2
#define OP_WRITE	3
#define OP_TRANSMIT	4

	u8_t			op;
	
	//                        | thread_A |
	// -->> r_io_2,r_io_1 -->>|          |-->> r_io_1,r_io_2 -->>
	//					      | thread_B |
	// Ͷ�ݶ�io���б�ţ��������������⣬����Ͷ��io�����Ⱥ��ȡ����Ͷ�ݵ�������ǰ(������ʾ˳����)
//	u64_t			id;
	
	// 0����Ϊ����� 1����Ϊ�ͻ���
	u8_t			cc;
	
	// ��Ϊ�ͻ���
	sockaddr_in		inaddr;// ���ӵ��Զ�ip

	// ����Ҫ�����ر�����ʱ������ΪTRUE����Ͷ��OP_READ
	u8_t			closing;
}io_t;

//------------------------------------------------------------------------
// listen_t
//------------------------------------------------------------------------
typedef struct _listen_t
{
	// listenfd
	SOCKET		fd;

	ulong_t		ipaddr;					// �����ֽ���ipaddr
	u16_t		port;					// �����ֽ���port
	
	// ��Ϊ�ͻ���
	sockaddr_in	inaddr;// ���ӵ��Զ�ip

//	_listen_t*	next;

}listen_t,sock_t;

//------------------------------------------------------------------------
// connect_t/client_t
//------------------------------------------------------------------------
typedef struct _connect_t
{
	SOCKET		fd;

	u16_t		af;

	// ����/Զ��ip
	ulong_t		l_ipaddr,r_ipaddr;		// �����ֽ���ipaddr
	u16_t		port; 					// �����ֽ���port

	// ��ǰͶ�ݶ�io���кţ��������������⣬����Ͷ��io�����Ⱥ��ȡ����Ͷ�ݵ�������ǰ
//	u64_t		id;
	
	// ��ǰ��ȡ�����б�ţ��������������⣬����Ͷ��io�����Ⱥ��ȡ����Ͷ�ݵ�������ǰ
	u64_t		cgid;
	
	// �����������ݰ�����
	u32_t		discard;

	// �ѽ��յ����ݰ�����
	u32_t		total;
	
	// �������ݰ�ȫ������id
	u32_t		gid;
	
	// �������ݰ���������pid
	u32_t		pid;
	
	// ����ʱ���
	time_t		ts;
/**
*	packet_pool_t * sp; // ���ݰ�����BUF��
*	packet_pool_t * rp; // ���ݰ�����BUF��(����ض�����������֪ͨ�û����Խ���)
*	map_packet_t  * hp; // ���ڻ���������(ǰ�������ȫ����ͷ�����ְ����ֻ�����ְ�ͷ�������������°�������°�ͷ��ȫ������)
*	map_packet_t  * bp; // ���ڻ��泬������
**/
}connect_t,client_t;

#endif // _XNET_H_INCLUDE_