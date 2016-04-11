/*
* 		��ͨ���ڴ�鼰�ڴ��ʵ��
*
* --------------------------------------------------------------------------------
* 		��
*			�ŵ㣺֧��׷�ӻ���
*			ȱ�㣺
*				1.��֧�ֲ�ͬ���ͽṹ������󹲴���һ������
*				2.�������ֽڷ��䣬�޷����ù��캯��������ʱ������Ϣ(dynamic_cast<TYPE>ʧЧ)��飬��֧�ֶ����麯������
* --------------------------------------------------------------------------------
*
* 1: ��׷�ӻ���
*	xBuffer<io_t>  g_pool(5,1024);
*	node_t <io_t>* t = g_pool.alloc();
*	t->data.buf = t->get_offset();
*	t->data.len = g_pool.get_offset_size();
*	g_pool.free(t);
*	g_pool.clear();
* 2: ��׷�ӻ���
*	xBuffer<xClient> g_pool(20,0);
*	node_t<xClient>* t = g_pool.alloc();
*	g_pool.free(t);
*	g_pool.clear();
*/

#ifndef _XBUFFER_H_INCLUDE_
#define _XBUFFER_H_INCLUDE_

#include "../xNet/xNetDll.h"
#include "../xThread/xThread.h"

// node_t<TYPE> �ڵ�
template < class TYPE > class XNET_API node_t
{
public:
	// ���ؽڵ����ݲ���
	inline TYPE * get_data(void)
	{
//		return (TYPE *) (char *)this;
		return (TYPE *) (char *)&this->data;
	}
	// ����׷�ӻ�����ַ
	inline char * get_offset(void)
	{
//		return (char *)(this + 1);
		return (char *)((char *)this + sizeof(node_t<TYPE>));
	}
public:
	// �ṹ�ֲ����£�
	// --------------------------
	// TYPE�ṹ | ָ��ṹ | ׷�ӻ���
	// --------------------------
	// ΪʲôҪ�����ֲ��أ������£�����windowsƽ̨���ص�io������ǰ������TYPE�ṹ��ǰû������
	// ָ��λ���ǹ̶��ģ�������TYPE�ṹ֮��׷�ӻ����Ƕ������ģ����������
	// ��ͬ���ڴ�ؿ���׷��buf��С��ͬ������ͬһ���ڴ�ض���׷��buf��С�����ǹ̶��ģ�xBufferEx��������

	TYPE data;
	node_t<TYPE> * prev,*next;
};

class ESMT::xMutex;
class ESMT::xEvent;

//------------------------------------------------------------------------
// block_node_t ��(һ)���ڴ�飬��N���ڵ�(sizeof(node_t<TYPE>) + offset)����
//------------------------------------------------------------------------
template < class TYPE > class XNET_API block_node_t
{
public:
	// @note: ���ؿ����ݲ���
	inline node_t<TYPE>* get_data(void)
	{
		return (node_t<TYPE> *)((char *)this + sizeof(block_node_t));
	}

	// ���뵥(һ)���ڴ��
	// header <block_node_t*&> : ��ͷ
	// tail   <block_node_t*&> : ��β
	// offset <u32_t>		  : ׷�ӻ����С sizeof(node_t<TYPE>) + offset
	// N      <u32_t>          : ���������Ľڵ�(sizeof(node_t<TYPE>) + offset)��
	static block_node_t * create(block_node_t*& header, block_node_t*& tail, u32_t offset, u32_t N)
	{
		//////////////////////////////////////////////////////////////////////////
		/// sizeof(block_node_t) + (sizeof(node_t<TYPE>) + offset) * N
		//////////////////////////////////////////////////////////////////////////
		block_node_t * block = (block_node_t *)new char[
				 sizeof(block_node_t) +
				(sizeof(node_t<TYPE>) + offset) * N];
		if ( ! block )
		{
			assert(FALSE);
			fprintf(stderr,"alloc/new failed,memery overflow.\n");
			return NULL;
		}
		block->prev = NULL;
		block->next = header;

		if( header )
		{
			header->prev = block;
			header = header->prev;
		}
		else tail = header = block;

		return block;
	}

	//�����ڴ��
	void clear(void)
	{
		block_node_t * block = this;

		while( block )
		{
			char * pbytes = (char *)block;
			block = block->next;
			delete [] pbytes;
		}
	}
public:
	block_node_t * prev,*next;
};

//------------------------------------------------------------------------
// xBuffer �ڴ��
//------------------------------------------------------------------------
template <class TYPE> class XNET_API xBuffer
{
public:
	// blocksize <u32_t> : �����ڴ�����������Ľڵ�(node_t<TYPE>)����ÿ������һ���ڴ��
	// offset    <u32_t> : ׷�ӻ����С sizeof(node_t<TYPE>) + offset
	xBuffer<TYPE>(u32_t blocksize = 20,u32_t offset = 0)
		:_readable_event(TRUE,FALSE),_empty_event(TRUE,FALSE)
	{
		_queue_size = 0;
		_head_node = _tail_node = NULL;
		_free_list   = NULL;
		_block_size  = blocksize;
		_offset_size = offset;
		_head_block = _tail_block = NULL;
	}

	// �ӳ��������ڴ�
	inline node_t<TYPE>* alloc(void)
	{
		return this->_create(NULL,NULL);
	}
	
	// ��ӣ��������ˣ�д������(������)
	// data <TYPE const*> : ����
	// buf  <char const*> : ׷������(�ӿ�Ԥ��)
	// len  <u32_t>       : ׷�����ݴ�С(�ӿ�Ԥ��)
	node_t<TYPE> * enqueue(TYPE const* data, char const* buf = NULL, u32_t len = 0)
	{
		node_t<TYPE>* node = NULL;
		// ͷ��д�룬β������
		this->_head_node_mutex.enter();
		if ( node = this->_create(NULL,this->_head_node) )
		{
			// �����ṹ��������
			memcpy_s(node->get_data(),  sizeof(TYPE), (void const*)data, sizeof(TYPE));
			
			// ����׷�ӻ�������
			memcpy_s(node->get_offset(), len, (void const*)buf , len);
			
			if( this->_head_node ) {
				_head_node->prev = node;
				this->_head_node = _head_node->prev;
			}
			else {
//				this->_tail_node_mutex.enter();
				this->_tail_node = this-> _head_node = node;
//				this->_tail_node_mutex.leave();
				
				// �����ݿɶ�
				__signaled(this->_readable_event);
				
				// ���зǿ�
//				__nonsignaled(this->_empty_event);
			}
			++this->_queue_size;
		}
		this->_head_node_mutex.leave();
		return node;
	}
	// @note: �ȴ�����
	node_t<TYPE> * wait_data(ulong_t timeout = INFINITE)
	{
		__wait_signal(this->_readable_event, timeout);
		return this->dequeue();
	}
	// @note: ���ӣ��������ˣ���������(������)
	node_t<TYPE>* dequeue(void)
	{
		node_t<TYPE> * node = NULL;
		
		// ͷ��д�룬β������
		this->_tail_node_mutex.enter();
		if ( this->_tail_node )
		{
			// ���Ĳ���ͷ�ڵ�
			if ( this->_tail_node != this->_head_node )
			{
				node = this->_tail_node;
				this->_tail_node = this->_tail_node->prev;
				node->prev = node->next = NULL;
				--this->_queue_size;
			}
			else
			{
				// ��ͷ�ڵ㣬��������д���ݣ���д���ٶ�(����ͬʱ��д��ͬ�ڵ�)
				// ��ʱ _tail_node �� _head_node Ϊ��ͬ�ڵ㣬Ӧ�˴�����
				this->_head_node_mutex.enter();
				
				node = this->_tail_node;
				this->_tail_node = this->_tail_node->prev;
				node->prev = node->next = NULL;
				--this->_queue_size;
				
				if ( node == this->_head_node ) {

					assert( !this->_tail_node );
					
					this->_head_node = this->_tail_node/* = NULL*/;
					
					// �����ѿ�
//					__signaled(this->_empty_event);
					
					// �����ݿɶ�
					__nonsignaled(this->_readable_event);
				}
				else {
					
					assert( this->_tail_node );
					
					this->_tail_node->next = NULL;
					
					// ����ָ��ͷ�ڵ㣬��Ҫ����!!!
//					this->_tail_node = this-> _head_node;
				}
				
				this->_head_node_mutex.leave();
			}
		}
		this->_tail_node_mutex.leave();
		return node;
	}
	inline u32_t queue_size() { return this->_queue_size; }

	// @note: �ͷ��ڴ浽��
	inline void free(node_t<TYPE>* node)
	{
// 		bzero(node->get_data(), sizeof(TYPE));
// 		bzero(node->get_offset(), this->_offset_size);
		bzero(node, sizeof(node_t<TYPE>) + this->_offset_size);

		this->_free_list_mutex.enter();
		node->prev = NULL;
		node->next = this->_free_list;
		if( this->_free_list ) _free_list->prev = node;
		this->_free_list = node;
		this->_free_list_mutex.leave();
	}
	
	// @note: �����ڴ��
	inline void clear(BOOL linear = FALSE)
	{
		// ����������ڵ�
		this->_free_list_mutex.enter();
		
		// ��������Ӳ���
		this->_head_node_mutex.enter();
		this->_head_node = NULL;
		this->_head_node_mutex.leave();
		
		if ( linear ) __wait_signal(this->_empty_event,INFINITE);
		// ��������Ӳ���
		this->_tail_node_mutex.enter();
		this->_tail_node = NULL;
		this->_tail_node_mutex.leave();

		this->_list_mutex.enter();
		SAFE_CLEAR(_head_block);
		_head_block = _tail_block = NULL;
		this->_list_mutex.leave();
		
		_free_list = NULL;
		this->_free_list_mutex.leave();
	}
	// @note: ����׷�ӻ����С
	inline u32_t get_offset_size(void)
	{
		return this->_offset_size;
	}
protected:
	// @param �����ڴ��
	node_t<TYPE> * _create(node_t<TYPE> * prev = NULL, node_t<TYPE> * next = NULL)
	{
		this->_free_list_mutex.enter();
		if( !this->_free_list )
		{
			this->_list_mutex.enter();
			block_node_t<TYPE>::create(this->_head_block, this->_tail_block, _offset_size, _block_size);
#if 0
			// _offset_size == 0
			node_t<TYPE> * node  = (node_t<TYPE> *)_head_block->get_data();
			node += ()_block_size - 1;

			for(u32_t cnt = this->_block_size; cnt > 0; --cnt, --node)
			{
				// 			bzero(node->get_data(), sizeof(TYPE));
				// 			bzero(node->get_offset(), this->_offset_size);
				bzero(node, sizeof(node_t<TYPE>) + this->_offset_size);
				node->prev = NULL;
				node->next = this->_free_list;
				_free_list = node;
			}
#else
			// _offset_size >  0
			char * node  = (char *)_head_block->get_data();
			node += (sizeof(node_t<TYPE>)+this->_offset_size) * (_block_size - 1);

			for(u32_t cnt = this->_block_size; cnt > 0;
				--cnt, node -= (sizeof(node_t<TYPE>)+this->_offset_size))
			{
				// 			bzero(((node_t<TYPE> *)node)->get_data(), sizeof(TYPE));
				// 			bzero(((node_t<TYPE> *)node)->get_offset(), this->_offset_size);
				bzero(node, sizeof(node_t<TYPE>) + this->_offset_size);

				((node_t<TYPE> *)node)->prev = NULL;
				((node_t<TYPE> *)node)->next = this->_free_list;
				_free_list = (node_t<TYPE> *)node;
			}
#endif
			this->_list_mutex.leave();
		}

		node_t<TYPE> * node = this->_free_list;
		_free_list = this->_free_list->next;
		node->prev = prev;
		node->next = next;
		this->_free_list_mutex.leave();
		return node;
	}

protected:
	
	u32_t _offset_size;			// ׷�ӻ����С sizeof(node_t<TYPE>) + offset
	u32_t _block_size;			// �����ڴ���������Ľڵ�(sizeof(node_t<TYPE>) + offset)����ÿ������һ���ڴ��
	u32_t _queue_size;
	
	node_t<TYPE> * _head_node;	// ˫����ͷ�ڵ�
	node_t<TYPE> * _tail_node;	// ˫����β�ڵ�
	ESMT::xMutex   _head_node_mutex;	// ͬ��ͷ��д���ݶ���
	ESMT::xMutex   _tail_node_mutex;	// ͬ��β�������ݶ���
	
	ESMT::xEvent   _readable_event; // �¼�֪ͨ������
	ESMT::xEvent   _empty_event;	// ���ݶ���Ϊ��

	node_t<TYPE> * _free_list;			// ���нڵ�����
	ESMT::xMutex   _free_list_mutex;	// ͬ����������
	
	block_node_t<TYPE> * _head_block;	// �ڴ��ͷ
	block_node_t<TYPE> * _tail_block;	// �ڴ��β
	ESMT::xMutex		 _list_mutex;	// ͬ��������
public:
	inline ~xBuffer<TYPE>(void)
	{
		this->clear();
	}
};

#endif // _XBUFFER_H_INCLUDE_
