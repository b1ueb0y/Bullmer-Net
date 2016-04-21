/*
* @note		��xReactor ��Ӧ������
*/

#ifndef _XREACTOR_H_INCLUDE_
#define _XREACTOR_H_INCLUDE_

#define MAX_PROCESS_THREAD_SIZE 2000

class ESMT::xThread;

class xClientFactory;
class xConnectionFactory;

//------------------------------------------------------------------------
// xReactor
//------------------------------------------------------------------------
class XNET_API xReactor
{
	friend class xNetObject;
public:
	//------------------------------------------------------------------------
	// xCollector
	//------------------------------------------------------------------------
	class XNET_API xCollector : public ESMT::xThread
	{
	public:
		xCollector(void * argument);
	private:
		u32_t __fastcall run(void * argument);
	};
	//------------------------------------------------------------------------
	// xDispatcher
	//------------------------------------------------------------------------
	class XNET_API xDispatcher : public ESMT::xThread
	{
	public:
		xDispatcher(void * argument);
		void __fastcall dispatch(xNetObject * object, e_event_t e);
	private:
		u32_t __fastcall run(void * argument);
	};
	//------------------------------------------------------------------------
	// xConsumer
	//------------------------------------------------------------------------
	class XNET_API xConsumer : public ESMT::xThread
	{
	public:
		xConsumer(void * argument);
	private:
		u32_t __fastcall run(void * argument);
	};
public:
	// @note����Ϊ TCP ����������Զ������
	// @param factory <xClientFactory*>����Ϊ TCP ����������
	// @param port				<u16_t>����ʼ�˿�
	// @param scope				<u32_t>��������Χ
	static void listenTCP(xClientFactory* factory, u16_t port = 6000, u32_t scope = 1);

	// @note: ��Ϊ TCP �ͻ�������Զ�̷�����
	// @param factory <xConnectionFactory*>����Ϊ TCP �ͻ��˹���
	// @param ipaddr			  <ulong_t>��Ŀ������ip
	// @param port					<u16_t>��������ʼ�˿�
	static void connectTCP(xConnectionFactory* factory, char const* ipaddr, u16_t port = 6000);

	// @param nMax <u32_t>����������󲢷��߳���(������ MAX_PROCESS_THREAD_SIZE)
	static int poll(u32_t nMax = 1);
	
	static xDispatcher * dispatcher(void);
	
	static u32_t total();
	static void increment();
	static void decrement();
};

#endif // _XREACTOR_H_INCLUDE_
