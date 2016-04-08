/*
* @note		: �û�������
* @author	��Andy.Ro
* @email	��Qwuloo@qq.com
* @date		��2014-06
* @mark		���õ����ĸ��ࣺxClient��xClientFactory��xConnection��xConnectionFactory ���ⲿ�̳�(λ��/xNet/Ŀ¼��)
*
*/
#ifndef _XUSER_SERVER_H_INCLUDE_
#define _XUSER_SERVER_H_INCLUDE_

// �õ������������
class xClient;		// ��Ϊ TCP ��������������ͻ������Ӷ���������
class xConnection;	// ��Ϊ TCP �ͻ��˽���������������Ӷ���������
class xClientFactory;		// ��ͻ������Ӵ��������(��Ϊ TCP ������)
class xConnectionFactory;	// ����������Ӵ��������(��Ϊ TCP �ͻ���)

class C2SClient;
class C2S_Server; // �û�������(��Ϊ TCP ������)

class S2PConnection;
class S2P_Server; // �û�������(��Ϊ TCP �ͻ���)


typedef C2S_Server UserServer;

//------------------------------------------------------------------------
// C2SClient
//------------------------------------------------------------------------
class C2SClient : public xClient
{
	friend class C2S_Server;
private:
	C2SClient(C2S_Server* factory);

	C2SClient* init(C2S_Server* factory);

protected:
	int onMade(void);
	int onLost(u32_t err);
	int onRecv(char const* buf, u64_t len);

public:
	u32_t _uid;
public:
	~C2SClient(void);
};

//------------------------------------------------------------------------
// C2S_Server
//------------------------------------------------------------------------
class C2S_Server : public xClientFactory
{
public:
	typedef std::map<u32_t,C2SClient*> map_id_client_t;
	typedef map_id_client_t::iterator itor_id_client_t;
	typedef map_id_client_t map_uID_C2S_client;
	typedef map_uID_C2S_client itor_uID_C2S_client;
public:
	C2S_Server(void);
	inline u32_t getsid(void) { return this->m_sid; }
	inline u32_t getpid(void) { return this->m_pid; }
public:
	xClient * create(void);
	void free (xClient * object);
	void onAdd(xClient * object);
	void onDel(xClient * object);
	void onMsg(xClient * object, char const* buf, u64_t len);
protected:
	u32_t m_sid;	// �û���id
	u32_t m_pid;	// ����ƽ̨��id
public:
	virtual ~C2S_Server(void);
};

//------------------------------------------------------------------------
// S2PConnection
//------------------------------------------------------------------------
class S2PConnection : public xConnection
{
	friend class S2P_Server;
private:
	S2PConnection(S2P_Server* factory);

	S2PConnection* init(S2P_Server* factory);

protected:
	int onMade(void);
	int onLost(u32_t err);
	int onRecv(char const* buf, u64_t len);

	int S2PRegister(void);
protected:

public:
	~S2PConnection(void);
};

//------------------------------------------------------------------------
// S2P_Server
//------------------------------------------------------------------------

class S2P_Server : public xConnectionFactory
{
	typedef std::map<SOCKET,xConnection* >  map_fd_conncection_t;
	typedef map_fd_conncection_t::iterator itor_fd_conncection_t;
public:
	S2P_Server(void);
	xConnection * create(void);
	void free (xConnection * object);
	void onAdd(xConnection * object);
	void onDel(xConnection * object);
	void onMsg(xConnection * object, char const* buf, u64_t len);
public:
	~S2P_Server(void);
};

#endif // _XUSER_SERVER_H_INCLUDE_