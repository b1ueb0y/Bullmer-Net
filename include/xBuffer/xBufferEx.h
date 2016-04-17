/*
*  �����ͨ�õ��ڴ����չʵ��
*
*	�ŵ㣺
*	    1.֧��׷�ӻ��棬����֧�������С(׷�������ж���׷�Ӷ�󻺴�,������ͬ�ṹ���;���׷��һ������1024�ֽ�)
*	    2.֧�ֲ�ͬ���ͽṹ������󹲴���һ������
*	ȱ�㣺�������ֽڷ��䣬��֧�ֶ����麯������
*
*	ע�⣺�������ֽڷ��䣬�޷����ù��캯��������ʱ������Ϣ(dynamic_cast<TYPE>ʧЧ)��飬����˿����Ӱ�첻ͬ�������ݵ�������ȡ!!!�����������ʱָ��Ψһ��ʶ���TYPE�ṹ�� typeid,
*		  ���ݳ�����ʱ���ɻ�ȡָ�� typeid ������͵�����!!!!�����⣬��������TYPE�ṹ���Ͳ�ͬ��
*		  ����size��С��ͬ�Ĳ�ͬTYPE��������ѹ��ͬһ�����У����Ծ��������� sizeof(TYPE) ��Ϊ typeid��
*		  ��Ȼ��Ԥ��֪��û����ͬsize��С�Ĳ�ͬTYPE�ṹ����ʱ�ǿ����õģ����������Ψһ��ʶ���ṹ��ľ�̬��Ա��Ϊ typeid
*
*  struct t1_t t1;
*  struct t2_t t2;
*
*  xBufferEx pool;
*  x_node_t * ptr;
*
*  u32_t typeid_1 = 1001,size_type_1 = sizeof(t1_t);
*  u32_t typeid_2 = 1002,size_type_2 = sizeof(t2_t);
*
*  // ѹ�� t1_t,t2_t ����ͬ���ͽṹ�������ݵ���ͬ����
*  pool.enqueue(&t1,typeid_1,size_type_1);
*  pool.enqueue(&t2,typeid_2,size_type_2);
*
*  // �ӳ��зֱ���ȡ t1_t,t2_t �ṹ��������
*  ptr = pool.dequeue(typeid_1,sizeof(t1_t));
*  while( ptr )
*  {
* 	   // ��ȡt1_t�ṹ����
*      ...
*      // ��ȡ�����ͷŽڵ�
*      pool.free(ptr,typeid_1,type_size_1,0);
* 
*      // ������ȡ��һ������
*      ptr = pool.dequeue(typeid_1,sizeof(t1_t));
*  }
*  
*  ptr = pool.dequeue(typeid_2,sizeof(t2_t));
*  while( ptr )
*  {
* 	   // ��ȡt2_t�ṹ����
*      ...
*      // ��ȡ�����ͷŽڵ�
*      pool.free(ptr,typeid_2,type_size_2,0);
* 
*      // ������ȡ��һ������
*      ptr = pool.dequeue(typeid_2,sizeof(t2_t));
*  }
*  
*  // ����ڴ�ز��������ͷ�
*  pool.clear();
*
*/

#ifndef _XBUFFEREX_H_INCLUDE_
#define _XBUFFEREX_H_INCLUDE_

#include "../xNet/xNetDll.h"
//#include "../Interface/xTypes.h"
#include "../xThread/xThread.h"

class x_node_t;
class x_block_node_t;

extern u32_t const ptr_size;
extern u32_t const ptr_sizex2;
extern u32_t const block_size;

class XNET_API x_node_t
{
public:
	inline void * get_data(void)
	{
		return (void *)(char *)this;
	}

	inline x_node_t ** get_prev_ptr(u32_t type_size)
	{
		return (x_node_t **)((char *)this+type_size);
	}

	inline x_node_t ** get_next_ptr(u32_t type_size)
	{
		return (x_node_t **)((char *)this+type_size + ptr_size);
	}

	inline char * get_offset(u32_t type_size)
	{
		return (char *)((char *)this + type_size + ptr_sizex2);
	}
protected:
	// ��xBufferһ�����ṹ�ֲ����£�
	// --------------------------
	// TYPE�ṹ | ָ��ṹ | ׷�ӻ���
	// --------------------------
	// ����Ϊʲô�����ֲ��鿴xBuffer�н��ͣ�����׸��
	// �����xBuffer�������TYPE��ָ�붼���أ�Ϊʲô�������أ���������£���Ϊ���ǿ�����һ���ڴ�ر��治ͬ���͵����ݣ�
	// �����أ�TYPE ���Ͳ��ܶ�����Ҳ������ģ�壬�׻�˵ģ��ģ�壬���ǰ����嶨�Ƶ���������ģ���ȱ�㻹�Ǻ����Եģ�
	// ����֧�ֲ�ͬsize�������ݣ�����ָ��λ�þͲ��̶ܹ����ˣ����䶯̬�仯�����Գ�Ա���嶼�����εģ���ν������Ϊ���Σ�
};

//------------------------------------------------------------------------
// x_block_node_t ��(һ)���ڴ�飬��N���ڵ�(sizeof(x_node_t<TYPE>) + offset)����
//------------------------------------------------------------------------

class XNET_API x_block_node_t
{
public:
	// @note: ���ؿ����ݲ���
	inline x_node_t* get_data(void)
	{
		return (x_node_t *)((char *)this + block_size);
	}

	// @note: ���뵥(һ)���ڴ��
	// @param header	  <x_block_node_t*&> : ��ͷ
	// @param tail		  <x_block_node_t*&> : ��β
	// @param type_size	  <u32_t>		     : TYPE�ṹ��С
	// @param offset_size <u32_t>		     : ׷�ӻ����С sizeof(x_node_t<TYPE>) + offset_size
	// @param N			  <u32_t>            : ���������Ľڵ�(sizeof(x_node_t<TYPE>) + offset_size)��

	static x_block_node_t * create(x_block_node_t*& header, x_block_node_t*& tail, u32_t type_size, u32_t offset_size, u32_t N)
	{
		// sizeof(x_block_node_t) + (type_size + 2 * sizeof(x_node_t*) + offset_size) * N
			
		x_block_node_t * block = (x_block_node_t *)new char[
				 block_size +
				(type_size  + ptr_sizex2 + offset_size) * N];
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

	//�����ڴ��
	void clear(void)
	{
		x_block_node_t * block = this;

		while( block )
		{
			char * pbytes = (char *)block;
			block = block->next;
			delete [] pbytes;
		}
	}
public:
	x_block_node_t * prev,*next;
};

// xBufferEx ��չ�ڴ��

class XNET_API xBufferEx
{
	
	typedef u64_t key_size,key_typeid;
	
	// this->_free_list[type_size + offset_size]
	typedef std::map<key_size, x_node_t*> map_size_node_t;
	typedef map_size_node_t::iterator    itor_size_node_t;
	
	// this->_head_node[typeid],this->_tail_node[typeid]
	typedef std::map<key_typeid, x_node_t*> map_typeid_node_t;
	typedef map_typeid_node_t::iterator    itor_typeid_node_t;
public:
	struct class_type_t
	{
		u64_t type_id;					// �ṹid(����Ǳ�ʶ�ṹ�ľ�̬��Ա��ַ)
		u32_t type_size, offset_size;	// �ṹ��С��׷�ӻ����С
	};
	
	//blocksize <u32_t> : �����ڴ�����������Ľڵ�(x_node_t)����ÿ������һ���ڴ��

	xBufferEx(u32_t blocksize = 20)
	{
		this->_head_node.clear();
		this->_tail_node.clear();
		this->_free_list.clear();
		_block_size  = blocksize;
		_head_block = _tail_block = NULL;
	}
	
	inline x_block_node_t *& get_head_block() { return this->_head_block; }
	inline x_block_node_t *& get_tail_block() { return this->_tail_block; }
	inline x_node_t * get_head_block_data() { return this->_head_block->get_data(); }
	inline x_node_t * get_tail_block_data() { return this->_tail_block->get_data(); }

	// �ӳ��������ڴ�
	inline x_node_t* alloc(u32_t type_size, u32_t offset_size)
	{
		return this->_create(type_size,offset_size,NULL,NULL);
	}

	// ���:д������
	// data  <TYPE const*> : ����
	// type_id     <u64_t> : Ψһ��ʶTYPE�ṹ����
	// type_size   <u32_t> : TYPE�ṹ�����С
	// offset_size <u32_t> : ׷�ӻ����С
	// buf   <char const*> : ׷�ӻ�������(�ӿ�Ԥ��)
	x_node_t * enqueue(void const* data, u64_t type_id, u32_t type_size, u32_t offset_size,char const* buf = NULL)
	{
		x_node_t* node = NULL;
		
		// ͷ��д�룬β������
		this->_head_node_mutex.enter();
		if ( node = this->_create(type_size, offset_size, NULL,this->_head_node[type_id]) )
		{
			// ��������
			memcpy_s(node->get_data(), type_size, (void const*)data, type_size);

			if( this->_head_node[type_id] ) {
				*this->_head_node[type_id]->get_prev_ptr(type_size) = node;
				this ->_head_node[type_id] = *_head_node[type_id]->get_prev_ptr(type_size);
			}
			else {
				this->_tail_node[type_id] = this-> _head_node[type_id] = node;
				// ���зǿ�
				__nonsignaled(this->_empty_event);
			}
			
			// �����ݿɶ�
			__signaled(this->_readable_event);
		}
		this->_head_node_mutex.leave();
		
		return node;
	}
	// �ȴ�����
	x_node_t * wait_data(u64_t type_id,u32_t type_size)
	{
		__wait_signal(this->_readable_event,INFINITE);
		return dequeue(type_id,type_size);
	}
	// ���ӣ���������(��ȡָ���ṹ���͵�����)
	// type_id     <u64_t> : Ψһ��ʶTYPE�ṹ����
	// type_size   <u32_t> : TYPE�ṹ�����С
	x_node_t* dequeue(u64_t type_id,u32_t type_size)
	{
		x_node_t* node = NULL;
		
		// ͷ��д�룬β������
		this->_tail_node_mutex.enter();
		if ( this->_tail_node[type_id] )
		{
			// ���Ĳ���ͷ�ڵ�
			if ( this->_tail_node[type_id] != this->_head_node[type_id] )
			{
				x_node_t* node = this->_tail_node[type_id];
				this->_tail_node[type_id] = *this->_tail_node[type_id]->get_prev_ptr(type_size);
				*node->get_prev_ptr(type_size) = *node->get_next_ptr(type_size) = NULL;
			}
			else
			{
				// ������ͷ�ڵ㣬��������д���ݣ���д���ٶ�(��Ϊ����ͬʱ��д��ͬ�ڵ�)
				__wait_signal(this->_readable_event,INFINITE);

				node = this->_tail_node[type_id];
//				this->_tail_node[type_id] = *this->_tail_node[type_id]->get_prev_ptr(type_size);
				*node->get_prev_ptr(type_size) = *node->get_next_ptr(type_size) = NULL;

				this->_head_node_mutex.enter(); assert( this->_tail_node[type_id] == this->_head_node[type_id] );

				if ( this->_tail_node[type_id] == this->_head_node[type_id]) {
					this->_tail_node[type_id] = this-> _head_node[type_id] = NULL;
					// �����ѿ�
					__signaled(this->_empty_event);
				}
				else {
					 this->_tail_node[type_id] = *this->_tail_node[type_id]->get_prev_ptr(type_size); assert( this->_tail_node[type_id] );
					*this->_tail_node[type_id]->get_next_ptr(type_size) = NULL;
				}

				this->_head_node_mutex.leave();
			}
			return node;
		}
		this->_tail_node_mutex.leave();
		
		return node;
	}

	// �ͷ��ڴ浽��
	// type_id     <u64_t> : Ψһ��ʶTYPE�ṹ����
	// type_size   <u32_t> : TYPE�ṹ�����С
	// offset_size <u32_t> : ׷�ӻ����С
	inline void free(x_node_t* node, u64_t type_id, u32_t type_size, u32_t offset_size)
	{
		u64_t size_key = type_size + offset_size;

		// bzero(node->get_data(), type_size);
		// bzero(node->get_offset(type_size), offset_size);
		bzero(node, type_size + ptr_sizex2 + offset_size);
		*node->get_prev_ptr(type_size) = NULL;
		*node->get_next_ptr(type_size) = this->_free_list[size_key];
		if( this->_free_list[size_key] )
			* _free_list[size_key]->get_prev_ptr(type_size) = node;
		this->_free_list[size_key] = node;
	}

	// �����ڴ��
	inline void clear(void)
	{
		this->_head_node.clear();
		this->_tail_node.clear();
		this->_free_list.clear();
		_head_block->clear();
		_head_block = _tail_block = NULL;
	}
protected:
	// �����ڴ��
	inline x_node_t * _create(u32_t type_size, u32_t offset_size, x_node_t * prev = NULL, x_node_t * next = NULL)
	{
		this->_free_list_mutex.enter();
		u64_t size_key = type_size + offset_size;
		if( !this->_free_list[size_key] )
		{
			this->_list_mutex.enter();
			x_block_node_t::create(this->_head_block, this->_tail_block, type_size, offset_size, _block_size);

			u64_t size_x_node_t = type_size + ptr_sizex2 + offset_size;

			char * node  = (char *)_head_block->get_data();
			node += size_x_node_t * (_block_size - 1);

			for(u32_t cnt = this->_block_size; cnt > 0;
				--cnt, node -= size_x_node_t)
			{
				// bzero(((x_node_t *)node)->get_data(), type_size);
				// bzero(((x_node_t *)node)->get_offset(type_size), offset_size);
				bzero(node, (u32_t)size_x_node_t);

				*((x_node_t *)node)->get_prev_ptr(type_size) = NULL;
				*((x_node_t *)node)->get_next_ptr(type_size) = this->_free_list[size_key];
				_free_list[size_key] = (x_node_t *)node;
			}
			this->_list_mutex.leave();
		}

		x_node_t * node = this->_free_list[size_key];
		this->_free_list[size_key] = *_free_list[size_key]->get_next_ptr(type_size);
		*node->get_prev_ptr(type_size) = prev;
		*node->get_next_ptr(type_size) = next;
		this->_free_list_mutex.leave();

		return node;
	}

protected:

	u32_t _block_size;		// �����ڴ���������Ľڵ�(sizeof(x_node_t) + offset)����ÿ������һ���ڴ��

	map_typeid_node_t _head_node;	// ˫����ͷ�ڵ�
	map_typeid_node_t _tail_node;	// ˫����β�ڵ�
	ESMT::xMutex   _head_node_mutex;	// ͬ��ͷ��д���ݶ���
	ESMT::xMutex   _tail_node_mutex;	// ͬ��β�������ݶ���

	ESMT::xEvent   _readable_event; // �¼�֪ͨ������
	ESMT::xEvent   _empty_event;	// ���ݶ���Ϊ��

	map_size_node_t   _free_list;		// ���нڵ�����
	ESMT::xMutex   _free_list_mutex;	// ͬ����������

	x_block_node_t * _head_block;	// �ڴ��ͷ
	x_block_node_t * _tail_block;	// �ڴ��β
	ESMT::xMutex	 _list_mutex;	// ͬ��������
public:
	inline ~xBufferEx(void)
	{
		this->clear();
	}
};

#endif // _XBUFFEREX_H_INCLUDE_
