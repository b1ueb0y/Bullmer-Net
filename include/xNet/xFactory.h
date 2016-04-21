/*
* @note		��xNetFactory ���������
*/

#ifndef _XFACTORY_H_INCLUDE_
#define _XFACTORY_H_INCLUDE_

#include "../xThread/xThread.h"
#include "xNet.h"

#include "../xBuffer/xQueue.h"

//#define _STL_QUEUE_

class ESMT::xSection;
class ESMT::xMutex;

class xNetObject ;
class xTransport ;
class xNetFactory;

//------------------------------------------------------------------------
// e_event_t
//------------------------------------------------------------------------
typedef enum _e_event_t {

	EVENT_READ		= 1,	// �������¼�
	EVENT_CLOSE,			// ���ӹر�
	EVENT_CLOSE_DESTORY		// ���ӹر����ٶ���
}e_event_t;

//------------------------------------------------------------------------
// xTransport
//------------------------------------------------------------------------

class XNET_API xTransport
{
	friend class xNetObject;
public:
	xTransport(void);
	void attach(xNetObject * object);
public:
	/*inline*/ int send(char const* buf, u64_t bytes);
	/*inline*/ BOOL peer_ipaddr(char * ipaddr, u32_t size);
	/*inline*/ BOOL self_ipaddr(char * ipaddr, u32_t size);
	/*inline*/ u16_t port();
private:
	xNetObject* _object;
public:
	~xTransport(void);
};

//------------------------------------------------------------------------
// xNetObject ���Ӷ������
//------------------------------------------------------------------------
class XNET_API xNetObject
{
public:
	xNetObject(void);

	inline void lock(void)		{ this->_object_cs.enter(); }
	inline void unlock(void)	{ this->_object_cs.leave(); }

	inline packet_pool_t * pp(void)	{ return &this->_pp; }
	inline packet_pool_t * rp(void)	{ return &this->_rp; }
	inline packet_pool_t * sp(void)	{ return &this->_sp; }
	inline map_packet_t  & bp(void)	{ return  this->_bp; }
	inline map_packet_t  & hp(void)	{ return  this->_hp; }
public:
	virtual int made(listen_t * sock, io_t * io)			/*= 0;*/{	return 0;	}	// ��������
	virtual int lost(u32_t err)								/*= 0;*/{	return 0;	}	// ��ʧ����
	virtual int recv(char const* buf, ulong_t bytes)		/*= 0;*/{	return 0;	}	// ��������
	virtual int send(char const* buf, u64_t bytes)			/*= 0;*/{	return 0;	}	// ��������

	/*inline*/ BOOL peer_ipaddr(char * ipaddr, u32_t size);
	/*inline*/ BOOL self_ipaddr(char * ipaddr, u32_t size);
	/*inline*/ u16_t port();

public:
	connect_t _object;
	ESMT::xSection _object_cs;
	xTransport _transport;
	packet_pool_t _sp; // ���ݰ�����BUF��
	packet_pool_t _pp; // �����������ݰ�����
	packet_pool_t _rp; // ���ݰ�����BUF��(����ض�����������֪ͨ�û����Խ���)
	map_packet_t  _hp; // ���ڻ���������(ǰ�������ȫ����ͷ�����ְ����ֻ�����ְ�ͷ�������������°�������°�ͷ��ȫ������)
	map_packet_t  _bp; // ���ڻ��泬������
	
	inline void enter_process_id_set() { this->_process_tid_cs.enter(); }
	inline void leave_process_id_set() { this->_process_tid_cs.leave(); }
	inline void set_process_id(u32_t tid)
	{
//		this->_process_tid_cs.enter();
		this->_process_tid = tid;
//		this->_process_tid_cs.leave();
	}
	inline u32_t get_process_id()
	{
		this->_process_tid_cs.enter();
		u32_t _tid = this->_process_tid;
		this->_process_tid_cs.leave();
		return _tid;
	}
	inline void set_event(e_event_t e )
	{
		this->_evt_id_cs.enter();
		if( e != this->_evt_id ) this->_evt_id = e;
		this->_evt_id_cs.leave();
	}
	inline e_event_t get_event(void   )
	{
		this->_evt_id_cs.enter();
		e_event_t e = this->_evt_id;
		this->_evt_id_cs.leave();
		return e;
	}
private:
	BOOL _in_queue;
	e_event_t _evt_id;
	ESMT::xSection _evt_id_cs;

	u32_t _process_tid;
	ESMT::xSection _process_tid_cs;
public:
#if 0
	static ESMT::xSection _p_list_cs;
#ifdef _STL_QUEUE_
	static std::queue<xNetObject*> _s_p_list; // �ȴ��������
#else
	static xQueue<xNetObject*> _s_p_list;
#endif
#endif
public:
	inline BOOL get_in_queue()			{ return this->_in_queue;			}
	inline void set_in_queue(BOOL state)	{ this->_in_queue = state;		}

#if 0
	static inline BOOL enqueue(xNetObject * object)
	{
		BOOL b_value = FALSE;
		// ���
		xNetObject::_p_list_cs.enter();
		if( !object->_in_queue )
		{
			b_value = TRUE;
			object->_in_queue = TRUE;
#ifdef _STL_QUEUE_
			xNetObject::_s_p_list.push(object);
#else
			xNetObject::_s_p_list.enqueue(object);
#endif
		}
		xNetObject::_p_list_cs.leave();
		return b_value;
	}
	static inline xNetObject * dequeue()
	{
		// ���ӣ�FIFO �Ƚ��ȳ�
		xNetObject * ptr = NULL;
		
		xNetObject::_p_list_cs.enter();
		if ( !xNetObject::_s_p_list.empty() )
		{
#ifdef _STL_QUEUE_
			ptr = xNetObject::_s_p_list.front();
			xNetObject::_s_p_list.pop();
#else
			q_node_t<xNetObject*>* s = xNetObject::_s_p_list.dequeue();
			ptr = s->get_data();
			xNetObject::_s_p_list.free(s);
#endif
			ptr->_in_queue = FALSE;
		}
		xNetObject::_p_list_cs.leave();

		return ptr;
	}
#endif
public:
	void       freepool(void);
	void    closesocket(void);
	virtual ~xNetObject(void);
};

//------------------------------------------------------------------------
// xNetFactory ����������
//------------------------------------------------------------------------
class XNET_API xNetFactory
{
public:
	xNetFactory(void);
public:
#if 0
	class xDispatcher
	{
	public:
		xDispatcher() {}
		virtual void dispatch(xNetObject * object)	= 0;
		virtual ~xDispatcher() {}
	};
	virtual xDispatcher * dispatcher(void)			= 0;
	virtual xNetObject * object(SOCKET fd)			= 0;
#endif
public:
 	virtual size_t    count(void)					= 0;
public:
	virtual ~xNetFactory(void);
};

#ifdef __cplusplus
extern "C" {
#endif

// @note��֪ͨ���ջص�����
typedef int (*recv_callback_func)(xNetObject* pthis, char const* buf, u64_t bytes);

// @note��֪ͨ���ͻص�����
typedef int (*send_callback_func)(xNetObject* pthis, char const* buf, ulong_t bytes);

// @note���������һ�����������ֳ�һ���������ݰ�(���ݿ����ϰ�ͷ)��֪ͨ�û��ص�����
// @mark�����Ͷˣ����/�ְ������������ݲ�ֳ�һ���������ݰ�(�������ϰ�ͷ) -> ����֪ͨ���ͻص�(��������)
XNET_API int split_packet(char const* buf, u64_t bytes,
					    send_callback_func callback_send,
					    xNetObject* pthis);

// @note��ճ�����룬���(ǰ����ƴ��)
// @mark�����նˣ��������� -> ճ�����룬���(ǰ����ƴ��) -> ƴ��/�����ȥ�������ݰ���ͷ��ƴ���������� -> ֪ͨ���ջص�(�������)
XNET_API int separate_continuous_packet(char const* buf, ulong_t bytes,
									  recv_callback_func callback_recv,
									  xNetObject* pthis);

#ifdef __cplusplus
}
#endif

#endif // _XFACTORY_H_INCLUDE_
