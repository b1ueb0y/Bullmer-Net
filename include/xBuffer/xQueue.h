/*
* @note		������
* @author	��Andy.Ro
* @email	��Qwuloo@qq.com
* @date		��2014-06
* @copyright(C) ��Ȩ����������
*
*/
#ifndef _XQUEUE_H_INCLUDE_
#define _XQUEUE_H_INCLUDE_

#include "../xNet/xNetDll.h"

//------------------------------------------------------------------------
// node_t<TYPE> �ڵ�
//------------------------------------------------------------------------
template < class TYPE > class /*XNET_API*/ q_node_t
{
public:
	inline TYPE get_data(void)
	{
		return this->data;
	}
public:
	TYPE data;
	q_node_t<TYPE> * prev,*next;
};

//------------------------------------------------------------------------
// block_q_node_t ��(һ)���ڴ�飬��N���ڵ� sizeof(q_node_t<TYPE>) ����
//------------------------------------------------------------------------
template < class TYPE > class /*XNET_API*/ block_q_node_t
{
public:
	// @note: ���ؿ����ݲ���
	inline q_node_t<TYPE>* get_data(void)
	{
		return (q_node_t<TYPE> *)((char *)this + sizeof(block_q_node_t));
	}

	// @note: ���뵥(һ)���ڴ��
	// @param header <block_q_node_t*&> : ��ͷ
	// @param tail   <block_q_node_t*&> : ��β
	static block_q_node_t * create(block_q_node_t*& header, block_q_node_t*& tail, u32_t N)
	{
		//////////////////////////////////////////////////////////////////////////
		/// sizeof(block_q_node_t) + sizeof(q_node_t<TYPE>) * N
		//////////////////////////////////////////////////////////////////////////
		block_q_node_t * block = (block_q_node_t *)new char[
			sizeof(block_q_node_t) + sizeof(q_node_t<TYPE>) * N];
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

	// @note: �����ڴ��
	void clear(void)
	{
		block_q_node_t<TYPE> * block = this;

		while( block )
		{
			char * pbytes = (char *)block;
			block = block->next;
			delete [] pbytes;
		}
	}
public:
	block_q_node_t * prev,*next;
};

//------------------------------------------------------------------------
// xQueue
//------------------------------------------------------------------------
template <class TYPE> class /*XNET_API*/ xQueue
{
public:
	xQueue<TYPE>(u32_t blocksize = 20);

	// @note�����
	void enqueue(TYPE& data)
	{
		q_node_t<TYPE>* node = NULL;
		
		this->_head_node_mutex.enter();
		if ( node = this->_create(NULL,this->_head_node) )
		{
			memcpy_s(&node->data, sizeof(TYPE), &data, sizeof(TYPE));
			
			if( this->_head_node ) {
				_head_node->prev = node;
				this->_head_node = _head_node->prev;
			}
			else {
				this->_tail_node_mutex.enter();
				this->_tail_node = this-> _head_node = node;
				this->_tail_node_mutex.leave();
			}
			++this->_queue_size;
		}
		this->_head_node_mutex.leave();
	}
	
	inline BOOL empty() { return 0 == this->_queue_size; }

	// @note������
	q_node_t<TYPE> * dequeue(void)
	{
		q_node_t<TYPE> * node = NULL;
		
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

	// @note: �ͷ��ڴ浽��
	inline void free(q_node_t<TYPE> * node)
	{
		bzero(node, sizeof(q_node_t<TYPE>));

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
protected:
	// @param �����ڴ��
	q_node_t<TYPE> * _create(q_node_t<TYPE> * prev = NULL, q_node_t<TYPE> * next = NULL);

public:
	
	u32_t _block_size;			// �����ڴ���������Ľڵ� sizeof(q_node_t<TYPE>) ����ÿ������һ���ڴ��
	u32_t _queue_size;
	
	q_node_t<TYPE> * _head_node;	// ˫����ͷ�ڵ�
	q_node_t<TYPE> * _tail_node;	// ˫����β�ڵ�
	ESMT::xSection   _head_node_mutex;	// ͬ��ͷ��д���ݶ���
	ESMT::xSection   _tail_node_mutex;	// ͬ��β�������ݶ���

	q_node_t<TYPE> * _free_list;				// ���нڵ�����
	ESMT::xMutex   _free_list_mutex;	// ͬ����������
	
	block_q_node_t<TYPE> * _head_block;	// �ڴ��ͷ
	block_q_node_t<TYPE> * _tail_block;	// �ڴ��β
	ESMT::xMutex	 _list_mutex;	// ͬ��������
public:
	inline ~xQueue(void)
	{
		this->clear();
	}
};

template < class TYPE >
inline xQueue<TYPE>::xQueue(u32_t blocksize /* = 20 */)
{
	_queue_size = 0;
	_head_node = _tail_node = NULL;
	_free_list   = NULL;
	_block_size  = blocksize;
	_head_block = _tail_block = NULL;
}

template < class TYPE >
inline q_node_t<TYPE>* xQueue<TYPE>::_create(q_node_t<TYPE> * prev, q_node_t<TYPE> * next)
{
	this->_free_list_mutex.enter();
	if( !this->_free_list )
	{
		this->_list_mutex.enter();
		block_q_node_t<TYPE>::create(this->_head_block, this->_tail_block, _block_size);
		
		q_node_t<TYPE> * node  = (q_node_t<TYPE> *)_head_block->get_data();
		node += _block_size - 1;
		
		for(u32_t cnt = this->_block_size; cnt > 0; --cnt, --node)
		{
			bzero(node, sizeof(q_node_t<TYPE>));
			node->prev = NULL;
			node->next = this->_free_list;
			_free_list = node;
		}
		this->_list_mutex.leave();
	}
	
	q_node_t<TYPE> * node = this->_free_list;
	_free_list = this->_free_list->next;
	node->prev = prev;
	node->next = next;
	this->_free_list_mutex.leave();
	return node;
}

#endif // _XQUEUE_H_INCLUDE_