/*
* @note		��iocp������
* @author	��Andy.Ro
* @mark		�������ࣺxNetFactory��xClient��xConnection ���ⲿ�̳�(λ��/xNet/Ŀ¼��)
*/

#ifndef _XIOCP_H_INCLUDE_
#define _XIOCP_H_INCLUDE_

#ifdef MAX_THREAD
#undef MAX_THREAD
#endif

#ifndef WSA_MAXIMUM_WAIT_EVENTS
#define WSA_MAXIMUM_WAIT_EVENTS 64
#endif

#define MAX_THREAD		1
#define mAX_LISTEN		WSA_MAXIMUM_WAIT_EVENTS // �������˿���
#define MAX_ACCEPT		150						// ��󲢷�������

class ESMT::xThread;
class ESMT::xThreadPool;
class ESMT::xEvent;

class xClientFactory;

// �����쳣�жϼ��
// http://blog.csdn.net/ghosc/article/details/5718978

//------------------------------------------------------------------------
// iocp������(����ģʽ)
//------------------------------------------------------------------------
class XNET_API xIocp
{
	typedef xBuffer<io_t> io_pool_t;
	typedef std::map<u32_t ,listen_t*> map_fd_listen_t;
//	typedef std::map<SOCKET,listen_t*> map_fd_listen_t;
	typedef map_fd_listen_t::iterator itor_fd_listen_t;
	typedef std::map<SOCKET,node_t<io_t>*> map_fd_io_t;
	typedef map_fd_io_t::iterator         itor_fd_io_t;
	typedef std::map<u32_t,ESMT::xEvent*> map_idx_event_t;
	typedef map_idx_event_t::iterator    itor_idx_event_t;
public:
	// �����̣߳�����������������¼�(FD_ACCEPT)
	class XNET_API xListener: public ESMT::xThread
	{
	public:
		u32_t __fastcall run(void * argument);
	};
	// �����߳�
	class XNET_API xWorker: public ESMT::xThread
	{
	public:
		xWorker(void * argument);
		u32_t __fastcall run(void * argument);
	};
public:
	static xIocp * shareObject(xClientFactory* factory);
private:
	// @note: ָ������������
	// @param service <xClientFactory*> : ����������
	//
	// @example:
	//	UserServer factory;
	//	xIocp iocp(&factory);
	//
	xIocp(xClientFactory* factory);
public:
	// @note����Ϊ TCP ����������Զ������
	// @param port	<u16_t> : ��ʼ�˿�
	// @param scope	<u32_t> : ������Χ
	void listenTCP(u16_t port = 6000, u32_t scope = 1);
	void stop(void) { this->_shutdown = TRUE; }
	// @note: �������м����׽��ֶ��󼰶�Ӧ���������¼�
	void close_all_listen(void);
	
	// @note: ���δ��acceptExͶ�ݣ�������Ӧ�µ���������
	void clear_pending_accept(void);
	// @note: �ر��������Ӷ���(���������ر�)[�����ر�]
	void notify_close_all(void);
	// @note: accept/recv/send io buf Ͷ��
	int post_accept_buf(SOCKET fd);
	int post_recv_buf  (SOCKET fd);
	int post_send_buf  (SOCKET fd, char const* buf, ulong_t len);
	int post_close_buf (SOCKET fd);
	
	// @note: accept/recv/send io buf �ͷ�
	void free_accept_buf(node_t<io_t>* io);
	void free_recv_buf  (node_t<io_t>* io);
	void free_send_buf	(node_t<io_t>* io);

protected:
	// @note: �˳����й����߳�
	void _end_workers(void);

	// @note: ����ָ���˿ڷ�Χ
	// @param port	<u16_t> : ��ʼ�˿�
	// @param scope	<u32_t> : ������Χ
	int _listening(u16_t port = 6000, u32_t scope = 1);

	// @note: io����
	// @param sock	<listen_t*>     : �����׽���(listenfd)����
	// @param io	<node_t<io_t>*>	: io����
	// @param bytes	<u32_t>	        : �����ֽ���
	// @param err	<u32_t>	        : �������
	int _handle_io(listen_t* sock, node_t<io_t>* io, ulong_t bytes, u32_t err);

	// @note: Ͷ��AcceptEx
	// @param sock	<SOCKET>        : �����׽���(listenfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_accept(SOCKET fd, node_t<io_t>* io);
	
	// @note: Ͷ��WSARecv
	// @param fd	<SOCKET>        : �ͻ��׽���(clientfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_recv  (SOCKET fd, node_t<io_t>* io);
	
	// @note: Ͷ��WSASend
	// @param fd	<SOCKET>        : �ͻ��׽���(clientfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_send  (SOCKET fd, node_t<io_t>* io);

	// @note: ����ָ��ip��ַ�Ͷ˿�
	// @param ipaddr <ulong_t>: �����ֽ���ipaddr
	// @param port   <u16_t>  : �����ֽ���port
	int _listening_one(ulong_t ipaddr, u16_t port);
private:
	BOOL _shutdown;
	HANDLE _hComPort;					// ��ɶ˿�
	u16_t _port;						// ��ʼ�˿�
	u32_t _scope_port;					// ������Χ
	map_fd_listen_t _listen_list;
	// �Ѿ�Ͷ�ݵ�δ����AcceptEx io �б�
	map_fd_io_t _pending_accps;
public:
	// ����FD_ACCEPT/FD_CLOSE�����¼�����⵽�����������Զ�Ͷ��AcceptEx����Ӧ���ӣ�
	// ֮ǰ�ǳ�ʼͶ�ݹ̶���������AcceptEx�����ܿͻ����ӣ����ڸ�Ϊ�Զ����Ͷ�ݷ�ʽ
	WSAEVENT _net_event[mAX_LISTEN];
	ESMT::xMutex _event_list_cs;

	xListener _listener;				// �������߳�
	ESMT::xThreadPool _workers;			// �������̳߳�
	xClientFactory * _factory;			// ����������
	ESMT::xSection	 _factory_cs;
public:
	BOOL _init_LPFN_PTR(void);
	LPFN_ACCEPTEX _lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS _lpfnGetAcceptExSockAddrs;
public:
	~xIocp(void);
};

#endif // _XIOCP_H_INCLUDE_
