/*
* @note		��ͬ���¼�/�ź���/����/�ٽ���/�߳�/�̳߳�ʵ��(win32/linux)����֧��unicode
*/

#ifndef _XTHREAD_H_INCLUDE_
#define _XTHREAD_H_INCLUDE_

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else

#include <sched.h>
#include <pthread.h>
#include <semaphore.h> // POSIX�ź���(�û�̬)
//#include <sys/sem.h>   // SYSTEM V�ź���(�û�̬)
#include <sys/types.h>
#include <unistd.h>
#define INFINITE 0xFFFFFFFF
#endif

#include "../Interface/xTypes.h"

#ifdef WIN32
typedef u32_t pthread_t;
#endif

namespace ESMT {

	class xSyncObject;
	class xEvent;					// �¼��ں˶���
	class xSemaphore;				// �ź����ں˶���linux �½�ʵ����POSIX�ź���
	class xMutex;					// �����ں˶���
	class xSection;					// �ٽ����û�����

	class xThread;					// �߳���
	class xThreadPool;				// �̳߳�

	//------------------------------------------------------------------------
	// xSyncObject �ӿ���
	//------------------------------------------------------------------------
	class XNET_API xSyncObject
	{
	public:
		/*XNET_API*/ xSyncObject(void);
	
	public:
		// @note: ����
		virtual BOOL lock(void)							/*= 0;*/ { return FALSE; }
		
		// @note: ��������̣߳�ֱ�����ں˶��󱻽���
		// @param timeout <ulong_t> : �ȴ�ʱ��(����)
		virtual BOOL wait(ulong_t timeout = INFINITE)	/*= 0;*/ { return FALSE; }
		
		// @note: ����
		virtual BOOL Unlock(void)						/*= 0;*/ { return FALSE; }
		
		// @note: ռ����Դ���������
		virtual BOOL enter(void)						/*= 0;*/ { return FALSE; }

		// @note: �뿪���ͷ�ռ����Դ
		virtual BOOL leave(void)						/*= 0;*/ { return FALSE; }

	public:
		/*XNET_API*/ virtual ~xSyncObject(void);
	};

	//------------------------------------------------------------------------
	// xEvent ���߳��¼�ͬ��
	// linux �²ο���http://blog.csdn.net/ffilman/article/details/4871920
	//------------------------------------------------------------------------
	class XNET_API xEvent : public xSyncObject
	{
#define		__signaled(e)		( (e).unlock() )
#define		__nonsignaled(e)	( (e).lock()   )
#define		__wait_signal(e,s)	( (e).wait(s)  )
	public:
		/*XNET_API*/ xEvent(
			BOOL manualreset = FALSE, BOOL initstate = FALSE
#ifdef WIN32
		   ,LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpsa = NULL
#else
		   ,u32_t pshared = PTHREAD_PROCESS_PRIVATE
		   ,u32_t type = PTHREAD_MUTEX_TIMED_NP
#endif
			);
	public:
		// @note: �����¼�δ����(non-signaled)״̬[�˹�����(manual-reset)�¼�]
		/*inline*/ BOOL lock();
		
		// @note: ��������̣߳�ֱ�����¼���������(signaled)
		// @param timeout <ulong_t> : �ȴ�ʱ��(����)
		/*inline*/ BOOL wait(ulong_t timeout = INFINITE);
		
		// @note: �����¼�����(signaled)״̬
		/*inline*/ BOOL unlock();
	
	public:
#ifdef WIN32
		HANDLE _event;
#else
#if 0
		typedef struct _cond_check_t
		{
			typedef BOOL (*cond_check_callback_func)(void * args);
			cond_check_callback_func _handler;
			void * _args;
		}cond_check_t;
		cond_check_t _cond_check;
#endif
		pthread_mutex_t _mutex;
		pthread_cond_t _cond;
		BOOL _manual_reset,_signaled;
#endif
	public:
		/*XNET_API*/ ~xEvent(void);
	};

	//------------------------------------------------------------------------
	// xSemaphore�����ƶ��̶߳�ͬһ������Դ�Ĳ�������
	// linux �²ο���http://blog.csdn.net/qinxiongxu/article/details/7830537
	//------------------------------------------------------------------------
	class XNET_API xSemaphore : public xSyncObject
	{
	public:
		// @note:�����ź��ں˶��� 
		// @param initvalue <ulong_t> : ��ʼ���÷�����Դ����
		// @param maxvalue  <ulong_t> : ������������Դ������ָ���������ʹ�����Դ������߳���
		/*XNET_API*/ xSemaphore(ulong_t initvalue = 1
#ifdef WIN32
			,ulong_t maxvalue = 1
#endif
			,char const * name = NULL);
	
	public:	
		// @note: ռ����Դ���������
		/*inline*/ BOOL enter();
		
		// @note: �뿪���ͷ�ռ����Դ
		/*inline*/ BOOL leave();
	
	public:
#ifdef WIN32
		HANDLE _sem;
#else
		sem_t  _sem,*_p_sem;
		std::string _S_NAME;
#endif
	public:
		/*XNET_API*/ ~xSemaphore(void);
	};

	//------------------------------------------------------------------------
	// xMutex ���ƶ��̶߳�ͬһ������Դ�Ļ������
	//------------------------------------------------------------------------
	class XNET_API xMutex : public xSyncObject
	{
	public:
		/*XNET_API*/ xMutex(
#ifdef WIN32
			 LPCTSTR lpName = NULL, BOOL bInitOwner = FALSE, LPSECURITY_ATTRIBUTES lpsa = NULL
#else
			 u32_t pshared = PTHREAD_PROCESS_PRIVATE
			,u32_t type = PTHREAD_MUTEX_TIMED_NP
#endif
			);
	
	public:
		// @note: ռ����Դ���������
		/*inline*/ BOOL enter();
		
		// @note: �뿪���ͷ�ռ����Դ
		/*inline*/ BOOL leave();
	
	public:
#ifdef WIN32
		HANDLE          _mutex;
#else
		pthread_mutex_t _mutex;
#endif
	public:
		/*XNET_API*/ ~xMutex(void);
	};

	//------------------------------------------------------------------------
	// xSection�����̷߳����ٽ���Դ
	//------------------------------------------------------------------------
	class XNET_API xSection : public xSyncObject
	{
	public:
		/*XNET_API*/ xSection(
#ifdef WIN32
#else
			 u32_t pshared = PTHREAD_PROCESS_PRIVATE
			,u32_t type = PTHREAD_MUTEX_TIMED_NP
#endif
			);
	
	public:
		// @note: ռ����Դ���������
		/*inline*/ BOOL enter();

		// @note: �뿪���ͷ�ռ����Դ
		/*inline*/ BOOL leave();
	
	private:
#ifdef WIN32
		CRITICAL_SECTION _cs;
#else
		pthread_mutex_t  _cs;
#endif
	public:
		/*XNET_API*/ ~xSection(void);
	};

	class xThreadPool;

	//------------------------------------------------------------------------
	// xThread �߳���
	//------------------------------------------------------------------------
	class XNET_API xThread
	{
		friend class xThreadPool;
	public:
		/*XNET_API*/ xThread(void);
		/*XNET_API*/ xThread(void * argument
#ifndef WIN32
			,BOOL detach = FALSE
			,u32_t scope = PTHREAD_SCOPE_SYSTEM
			,u32_t inherit = PTHREAD_EXPLICIT_SCHED
			,u32_t policy = SCHED_OTHER
#endif
			,u32_t priority = 0
			);
		/*virtual*/ BOOL stop() { return this->_done = TRUE;    }
	public:
		inline HANDLE operator *() const     { return _handler; }
		inline u32_t getid() const           { return (u32_t)_tid; }
		inline BOOL done() const             { return this->_done; }
		inline BOOL idle() const			 { return this->_idle; }
		inline void	setarg(void * argment)	 { this->_arg_cs.enter(); this->_arg = argment; this->_arg_cs.leave(); }
		inline void * getarg()
		{
			this->_arg_cs.enter();
			void * arg = this->_arg;
			this->_arg_cs.leave();
			return arg;
		}
	public:
		BOOL start(void * argument = NULL
#ifndef WIN32
			,BOOL detach = FALSE
			,u32_t scope = PTHREAD_SCOPE_SYSTEM     // PTHREAD_SCOPE_PROCESS��PTHREAD_SCOPE_SYSTEM
			,u32_t inherit = PTHREAD_EXPLICIT_SCHED // �̳��ԣ�PTHREAD_EXPLICIT_SCHED��PTHREAD_INHERIT_SCHED
			,u32_t policy = SCHED_OTHER             // ���Ȳ��ԣ�SCHED_FIFO��SCHED_RR��SCHED_OTHER ..
#endif
			,u32_t priority = 0
			);
		BOOL wait(u32_t timeout = INFINITE);
#ifndef WIN32
		void detach();
#endif
		BOOL setpriority(u32_t priority);
		u32_t getpriority();
		void suspend();
		void resume();
	protected:
		// @note��run()����ʱ�����޸����ȼ�
		BOOL onpriority();
	protected:
		void release();
	protected:
		virtual u32_t __fastcall run(void * argument) = 0;
	protected:
// 		typedef struct _thread_param_t
// 		{
// 			xThread * pthis;
// 			void * argument;
// 		}thread_param_t;
// 		thread_param_t _param;
		
		pthread_t      _tid;
		u32_t          _priority; // ���޸ĵ����ȼ�
		BOOL           _modify;   // �޸ı��
		BOOL           _done;
		BOOL		   _idle;	  // ���У�������
#ifdef WIN32
		HANDLE         _handler;
#endif
		void *	       _arg;
		ESMT::xSection _arg_cs;
	private:
		static u32_t __stdcall routine(void * param);
	public:
		/*XNET_API*/ virtual ~xThread(void);
	};

	//------------------------------------------------------------------------
	// xThreadPool �̳߳�
	//------------------------------------------------------------------------
	class XNET_API xThreadPool
	{
		typedef std::map<u32_t,xThread*> map_id_thread_t;
		typedef map_id_thread_t::iterator itor_id_thread_t;
	public:
		/*XNET_API*/ xThreadPool(void);

		xThread * pop_idle();
		void push_idle(xThread * thread);
		size_t idle_size(void);

		xThread * add(xThread * thread);
		void del(u32_t id);
		void clear(void);
		
		inline void enter() { this->_thread_pool_cs.enter(); }
		inline void leave() { this->_thread_pool_cs.leave(); }

		xThread * get(u32_t id);
		size_t size(void);
		
	public:
		/*XNET_API*/ ~xThreadPool(void);
	private:
		map_id_thread_t _thread_pool;
		ESMT::xSection	_thread_pool_cs;
		std::stack<xThread*> _idle_list;
		ESMT::xSection       _idle_list_cs;
	};
} // namespace ESMT

#endif // _XTHREAD_H_INCLUDE_
