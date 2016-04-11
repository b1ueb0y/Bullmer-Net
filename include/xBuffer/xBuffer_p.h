/*
* ͨ���ڴ�鼰�ڴ��ʵ��
*
* --------------------------------------------------------------------------------
*
*			�ŵ㣺֧��׷�ӻ���
*			ȱ�㣺
*				1.��֧�ֲ�ͬ���ͽṹ������󹲴���һ������
*				2.�������ֽڷ��䣬�޷����ù��캯��������ʱ������Ϣ(dynamic_cast<TYPE>ʧЧ)��飬��֧�ֶ����麯������
* --------------------------------------------------------------------------------
*
* 1: ��׷�ӻ���
*	xBuffer<io_t>  g_pool(5,1024);
*	p_node_t <io_t>* t = g_pool.alloc();
*	t->data.buf = t->get_offset();
*	t->data.len = g_pool.get_offset_size();
*	g_pool.free(t);
*	g_pool.clear();
* 2: ��׷�ӻ���
*	xBuffer<xClient> g_pool(20,0);
*	p_node_t<xClient>* t = g_pool.alloc();
*	g_pool.free(t);
*	g_pool.clear();
*/

#ifndef _XBUFFER_P_H_INCLUDE_
#define _XBUFFER_P_H_INCLUDE_

#include "../xNet/xNetDll.h"
#include "../xThread/xThread.h"

//------------------------------------------------------------------------
// p_node_t<TYPE> �ڵ�
//------------------------------------------------------------------------
class XNET_API p_node_t
{
public:
	// ���ؽڵ����ݲ���
	inline char* get_data(void)
	{
//		return (char *)(this + 1);
		return (char *)((char *)this + sizeof(p_node_t));
	}
public:
	// �ṹ�ֲ����£�
	// -------------------------
	// ָ��ṹ | ���ݳ��� | ׷�ӻ���
	// -------------------------
	p_node_t * prev,*next;
	u32_t           bytes; // ���ݳ���
};

class ESMT::xMutex;
class ESMT::xEvent;

//------------------------------------------------------------------------
// block_p_node_t ��(һ)���ڴ�飬��N���ڵ�(sizeof(p_node_t<TYPE>) + offset)����
//------------------------------------------------------------------------
class XNET_API block_p_node_t
{
public:
	// @note: ���ؿ����ݲ���
	inline p_node_t* get_data(void)
	{
		return (p_node_t *)((char *)this + sizeof(block_p_node_t));
	}

	// @note: ���뵥(һ)���ڴ��
	// @param header <block_p_node_t*&> : ��ͷ
	// @param tail   <block_p_node_t*&> : ��β
	// @param offset <u32_t>		  : ׷�ӻ����С sizeof(p_node_t<TYPE>) + offset
	// @param N      <u32_t>          : ���������Ľڵ�(sizeof(p_node_t<TYPE>) + offset)��
	static block_p_node_t * create(block_p_node_t*& header, block_p_node_t*& tail, u32_t offset, u32_t N)
	{
		//////////////////////////////////////////////////////////////////////////
		/// sizeof(block_p_node_t) + (sizeof(p_node_t<TYPE>) + offset) * N
		//////////////////////////////////////////////////////////////////////////
		block_p_node_t * block = (block_p_node_t *)new char[
				 sizeof(block_p_node_t) +
				(sizeof(p_node_t      ) + offset) * N];
		if ( ! block )
		{
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

	// @note: �����ڴ��
	void clear(void)
	{
		block_p_node_t * block = this;

		while( block )
		{
			char * pbytes = (char *)block;
			block = block->next;
			delete [] pbytes;
		}
	}
public:
	block_p_node_t * prev,*next;
};

//------------------------------------------------------------------------
// xBuffer_p �ڴ��
//------------------------------------------------------------------------
class XNET_API xBuffer_p
{
public:
	// @param blocksize <u32_t> : �����ڴ�����������Ľڵ�(p_node_t<TYPE>)����ÿ������һ���ڴ��
	// @param offset    <u32_t> : ׷�ӻ����С sizeof(p_node_t<TYPE>) + offset
	xBuffer_p(u32_t blocksize = 20,u32_t offset = 0)
		:_readable_event(TRUE,FALSE),_empty_event(TRUE,TRUE)
	{
		_queue_size = 0;
		_head_node = _tail_node = NULL;
		_free_list   = NULL;
		_block_size  = blocksize;
		_offset_size = offset;
		_head_block = _tail_block = NULL;
	}

	// @note: �ӳ��������ڴ�
	inline p_node_t * alloc(void)
	{
		return this->_create(NULL,NULL);
	}
	
	// @note: ��ӣ��������ˣ�д������(������)
	// @param data <TYPE const*> : ����
	// @param buf  <char const*> : ׷������(�ӿ�Ԥ��)
	// @param len  <u32_t>       : ׷�����ݴ�С(�ӿ�Ԥ��)
	p_node_t * enqueue(char const* buf = NULL, u32_t len = 0)
	{
		p_node_t * node = NULL;
		
		// ͷ��д�룬β������
		this->_head_node_mutex.enter();
		if ( node = this->_create(NULL,this->_head_node) )
		{
			// ʵ���������ݳ���
			node->bytes = len;
			
			// ����׷�ӻ�������
			memcpy_s(node->get_data(), len, (void const*)buf, len);
			
			if( this->_head_node ) {
				_head_node->prev = node;
				this->_head_node = _head_node->prev;
			}
			else {
				this->_tail_node_mutex.enter();
				this->_tail_node = this-> _head_node = node;
				this->_tail_node_mutex.leave();

				// �����ݿɶ�
//				__signaled(this->_readable_event);
				
				// ���зǿ�
//				__nonsignaled(this->_empty_event);
			}
			++this->_queue_size;
		}
		this->_head_node_mutex.leave();
		return node;
	}
	// @note: �ȴ�����
	p_node_t * wait_data(ulong_t timeout = INFINITE)
	{
//		__wait_signal(this->_readable_event, timeout);
		return this->dequeue();
	}
	// @note: ���ӣ��������ˣ���������(������)
	p_node_t * dequeue(void)
	{
		p_node_t * node = NULL;
		
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
//					__nonsignaled(this->_readable_event);
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
	
	// @note���Ƚ������ݰ��ڵ����б��
	// @param pa <p_node_t*>�������ݽڵ�
	// @param pb <p_node_t*>�����ȽϽڵ�
	typedef int (*compare_func_ptr) (p_node_t * pa, p_node_t * pb);
	// @note: ���ݱȽϹ�����ӵ��������
	void orderinsert(compare_func_ptr compare_ptr, char const* buf, u32_t len)
	{
		this->_head_node_mutex.enter();
		p_node_t * pre = NULL,*cur = _head_node,*ptr;
		
		// ptr�ڵ�Ϊ�����ݽڵ㣬cur�ڵ�Ϊ�����ڵ�
		if ( ptr = this->_create(NULL,NULL) )
		{
			// ʵ���������ݳ���
			ptr->bytes = len;
			
			// ����׷�ӻ�������
			memcpy_s(ptr->get_data(), len, (void const*)buf, len);

			while ( cur )
			{
				// ִ�бȽϹ���
				if( (*compare_ptr)(ptr, cur) > 0 )
				{
					break;
				}
				pre = cur;
				cur = cur->next;
			}
			// ��ǰ�ڵ�֮ǰ����
			if ( !pre )
			{
				// ����ͷ��
				ptr-> next = this->_head_node;
				_head_node = ptr;

				++this->_queue_size;
			}
			else
			{
				ptr->next = cur;
				pre->next = ptr;
				
				++this->_queue_size;
			}
		}
		this->_head_node_mutex.leave();
	}
	// @note: 
	inline p_node_t * top(void)
	{
		return this->_head_node;
	}
	// note: ������ͷ�ڵ�
	inline p_node_t * pop(void)
	{
		p_node_t * node  = this->_head_node;
		this->_head_node = node->next;
		node->next = node->prev = NULL;
		if( this->_head_node )
			this->_head_node->prev = NULL;
		--this->_queue_size;
		return node;
	}

	// @note: �ͷ��ڴ浽��
	inline void free(p_node_t * node)
	{
// 		bzero(node->get_data(), this->_offset_size);
		bzero(node, sizeof(p_node_t) + this->_offset_size);

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
//		this->_free_list_mutex.enter();
		
		// ��������Ӳ���
//		this->_head_node_mutex.enter();
		this->_head_node = NULL;
//		this->_head_node_mutex.leave();
		
//		if ( linear ) __wait_signal(this->_empty_event,INFINITE);
		// ��������Ӳ���
//		this->_tail_node_mutex.enter();
		this->_tail_node = NULL;
//		this->_tail_node_mutex.leave();

//		this->_list_mutex.enter();
		SAFE_CLEAR(_head_block);
		_head_block = _tail_block = NULL;
//		this->_list_mutex.leave();
		
//		__signaled(this->_empty_event);
//		__nonsignaled(this->_readable_event);

		_free_list = NULL;
//		this->_free_list_mutex.leave();
	}
	// @note: ����׷�ӻ����С
	inline u32_t get_offset_size(void)
	{
		return this->_offset_size;
	}
protected:
	// @param �����ڴ��
	inline p_node_t * _create(p_node_t * prev = NULL, p_node_t * next = NULL)
	{
		this->_free_list_mutex.enter();
		if( !this->_free_list )
		{
			this->_list_mutex.enter();
			block_p_node_t::create(this->_head_block, this->_tail_block, _offset_size, _block_size);
#if 0
			// _offset_size == 0
			p_node_t * node  = (p_node_t *)_head_block->get_data();
			node += ()_block_size - 1;

			for(u32_t cnt = this->_block_size; cnt > 0; --cnt, --node)
			{
				// 			bzero(node->get_data(), this->_offset_size);
				bzero(node, sizeof(p_node_t) + this->_offset_size);
				node->prev = NULL;
				node->next = this->_free_list;
				_free_list = node;
			}
#else
			// _offset_size >  0
			char * node  = (char *)_head_block->get_data();
			node += (sizeof(p_node_t)+this->_offset_size) * (_block_size - 1);

			for(u32_t cnt = this->_block_size; cnt > 0;
				--cnt, node -= (sizeof(p_node_t)+this->_offset_size))
			{
				// 			bzero(((p_node_t *)node)->get_data(), this->_offset_size);
				bzero(node, sizeof(p_node_t) + this->_offset_size);

				((p_node_t *)node)->prev = NULL;
				((p_node_t *)node)->next = this->_free_list;
				_free_list = (p_node_t *)node;
			}
#endif
			this->_list_mutex.leave();
		}

		p_node_t * node = this->_free_list;
		_free_list = this->_free_list->next;
		node->prev = prev;
		node->next = next;
		this->_free_list_mutex.leave();
		return node;
	}

public:
	
	u32_t _offset_size;			// ׷�ӻ����С sizeof(p_node_t<TYPE>) + offset
	u32_t _block_size;			// �����ڴ���������Ľڵ�(sizeof(p_node_t<TYPE>) + offset)����ÿ������һ���ڴ��
	u32_t _queue_size;
	
	p_node_t * _head_node;	// ˫����ͷ�ڵ�
	p_node_t * _tail_node;	// ˫����β�ڵ�
	ESMT::xSection   _head_node_mutex;	// ͬ��ͷ��д���ݶ���
	ESMT::xSection   _tail_node_mutex;	// ͬ��β�������ݶ���
	
	ESMT::xEvent   _readable_event; // �¼�֪ͨ������
	ESMT::xEvent   _empty_event;	// ���ݶ���Ϊ��

	p_node_t * _free_list;				// ���нڵ�����
	ESMT::xMutex   _free_list_mutex;	// ͬ����������
	
	block_p_node_t * _head_block;	// �ڴ��ͷ
	block_p_node_t * _tail_block;	// �ڴ��β
	ESMT::xMutex	 _list_mutex;	// ͬ��������
public:
	inline ~xBuffer_p(void)
	{
		this->clear();
	}
};

#endif // _XBUFFER_P_H_INCLUDE_
