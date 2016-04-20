/*
* @note		��iocp������
* @mark		�������ࣺxNetFactory��xClient��xConnection ���ⲿ�̳�(λ��/xNet/Ŀ¼��)
*
*/

#ifndef _XIOCPCLIENT_H_INCLUDE_
#define _XIOCPCLIENT_H_INCLUDE_

#ifdef MAX_THREAD
#undef MAX_THREAD
#endif
#define MAX_THREAD		2

class ESMT::xThread;
class ESMT::xThreadPool;
class ESMT::xEvent;

class xConnectionFactory;

//------------------------------------------------------------------------
// iocp�ͻ���(����ģʽ)
//------------------------------------------------------------------------
class XNET_API xIocpClient
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
	// �����߳�
	class XNET_API xWorker: public ESMT::xThread
	{
	public:
		xWorker(void * argument);
		u32_t __fastcall run(void * argument);
	};
public:
	static xIocpClient * shareObject(xConnectionFactory* factory);
private:
	// @note: ָ������������
	// @param service <xConnectionFactory*> : ����������
	//
	// @example:
	//	UserServer factory;
	//	xIocpClient iocp(&factory);
	//
	xIocpClient(xConnectionFactory* factory);
	void made(sock_t * sock, node_t<io_t>* io);
	void lost(node_t<io_t>* io, u32_t err);
	void recv(node_t<io_t>* io);
public:
	// @note: ��Ϊ TCP �ͻ�������Զ�̷�����
	// @param ipaddr <ulong_t> : Ŀ������ip
	// @param port	 <u16_t>   : ������ʼ�˿�
	// @param scope	 <u32_t>   : ���Ӷ˿ڷ�Χ
	void connectTCP(ulong_t ipaddr, u16_t port = 6000, u32_t scope = 1);
	void stop(void) { this->_shutdown = TRUE; }
	
	// @note: connect/recv/send io buf Ͷ��
	int post_connect_buf(sock_t * sock);
	int post_recv_buf   (SOCKET fd);
	int post_send_buf   (SOCKET fd, char const* buf, ulong_t len);
	int post_close_buf  (SOCKET fd);
	// @note: connect/recv/send io buf �ͷ�
	void free_connect_buf(node_t<io_t>* io);
	void free_recv_buf  (node_t<io_t>* io);
	void free_send_buf	(node_t<io_t>* io);

protected:
	// @note: �˳����й����߳�
	void _end_workers(void);

	// @note: ����ָ���˿ڷ�Χ
	// @param port	<u16_t> : ��ʼ�˿�
	// @param scope	<u32_t> : ������Χ
	int _connect(ulong_t ipaddr, u16_t port = 6000, u32_t scope = 1);
	
	// @note: io����
	// @param sock	<listen_t*>     : �����׽���(connectfd)����
	// @param io	<node_t<io_t>*>	: io����
	// @param bytes	<u32_t>	        : �����ֽ���
	// @param err	<u32_t>	        : �������
	int _handle_io(listen_t* sock, node_t<io_t>* io, ulong_t bytes, u32_t err);

	// @note: Ͷ��ConnectEx
	// @param sock	<listen_t*>     : �����׽���(connectfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_connect(SOCKET fd, node_t<io_t>* io);
	
	// @note: Ͷ��WSARecv
	// @param fd	<SOCKET>        : �����׽���(connectfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_recv   (SOCKET fd, node_t<io_t>* io);
	
	// @note: Ͷ��WSASend
	// @param fd	<SOCKET>        : �����׽���(connectfd)
	// @param io	<node_t<io_t>*>	: io_t�¶���
	int _post_send   (SOCKET fd, node_t<io_t>* io);

	// @note: ����ָ��ip��ַ�Ͷ˿�
	// @param ipaddr <ulong_t>: �����ֽ���ipaddr
	// @param port   <u16_t>  : �����ֽ���port
	int _connect_one(ulong_t ipaddr, u16_t port);
public:
	BOOL _shutdown;
	HANDLE _hComPort;					// ��ɶ˿�
	u16_t _port;						// ��ʼ�˿�
	u32_t _scope_port;					// �˿ڷ�Χ

	ESMT::xThreadPool _workers;			// �������̳߳�
	xConnectionFactory * _factory;		// ����������
	ESMT::xMutex         _factory_cs;
protected:
	BOOL _init_LPFN_PTR(void);
	LPFN_CONNECTEX _lpfnConnectEx;
	LPFN_TRANSMITFILE _lpfnTransmitFile;

public:
	~xIocpClient(void);
};

#endif // _XIOCPCLIENT_H_INCLUDE_ 
