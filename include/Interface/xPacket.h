/*
* @note		�����ݰ�Э��ṹ
*
* @mark		�����ݰ��շ����̣�
*				���Ͷˣ����/�ְ������������ݲ�ֳ�һ���������ݰ�(�������ϰ�ͷ) -> ����֪ͨ���ͻص�(��������)
*				���նˣ��������� -> ճ�����룬���(ǰ����ƴ��) -> ƴ��/�����ȥ�������ݰ���ͷ��ƴ���������� -> ֪ͨ���ջص�(�������)
*
*/

#ifndef _XPACKET_H_INCLUDE_
#define _XPACKET_H_INCLUDE_

#include "../xBuffer/xBuffer_p.h"

#pragma pack(push,1)

//------------------------------------------------------------------------
// packet_t ���ݰ�Э��ṹ�����Ͷ˺ͽ��ն�Э�������ȫһ��
//------------------------------------------------------------------------
typedef struct _packet_t
{
#define FMT_RAW		0
#define FMT_JSON	1
#define FMT_XML		2
#define FMT_TEXT	3
	//////////////////////////////////////////////////////////////////////////
	// ��ͷ(header)

	// 0  ~ 1 λ������(���ݲ���)��ʽ(0��raw|1��json|2��xml|3��text)�����������ԣ������ݰ������ʽ�϶���һ�µ�
	// 2      λ�����ݰ���ʼ��(0���ǿ�ʼ 1����ʼ)�����������ԣ���ǰ���Ƿ��ǵ�һ���������װ�
	// 3      λ�����ݰ�������(0���ǽ��� 1������)�����������ԣ���ǰ���Ƿ�������������β��
	// 4  ~ 31λ�����������������ݰ�����(total)
	// 32 ~ 47λ����ǰ����(���ݲ���)��С(len )����׷�ӻ��沿�ִ�ŵ�ʵ�����ݳ��ȣ������� MAX_PACKETSZ
	// 48 ~ 63λ��β������(���ݲ���)��С(tail)�������� MAX_PACKETSZ
	u64_t		flag;					// ����������������������ݽ��ն����Ի�������(�ֽڣ����ͣ�����)Ϊ��λ�ģ�
										// ��������ͷ��Ϣ������һ�����ͱ����ϣ�������Ϊ��ͷ�׳�Ա����߽��տɿ��ԣ�
										// ��ɢ�ڲ�ͬ��Ա�ϲ���ȫ������ֻ�յ����ֳ�Ա����
	
	u64_t		gid;					// ���Ͷ�ȫ��Ψһ��ʶ�����ݰ���������������(���1��ʼ)�����ն˴�������������
	
	u32_t		pid;					// Ψһ��ʶ���ݰ���������(���1��ʼ)������������ʱ��һ������ֲ�ɶ�����ݰ��� pid ��ͬ
	
	u32_t		id;						// ���������ԣ�����ظ����ݰ��������idΨһ������(���1��ʼ)��ƴ���������������(��� UDP ���ɿ�������˵����ȫ©��ĳ�����п��ܵ�)
	
	u64_t		offset;					// ��ǰ����(���ݲ���)��������ƫ��
	
	// ����ʱ���
	time_t		ts;
	
	// У���
	u16_t		checksum;
	
	//////////////////////////////////////////////////////////////////////////
	// ����(body-content)�������ݲ���		// ׷�ӻ��沿�֣���ַΪ (char *)this + sizeof(packet_header_t)

	// ����С������(�����ݴ�С������MAX_PACKETSZ��total=1)���ԣ����Ͷ�����ְ����ͣ����ն�����ƴ����һ�����ݰ����ɱ�ʾ���壺len
	// ���ڴ�������(�����ݴ�С�ѳ���MAX_PACKETSZ��total>1)���ԣ����Ͷ���Ҫ�ְ����ͣ����ն���Ҫƴ�����ɸ������ݰ�ƴ�ӳ����壺(total - 1) * len(MAX_PACKETSZ) + tail

}packet_t;

#pragma pack(pop)

// ֱ�ۿ���ʵ�������ݰ�ͷ�ṹ
typedef packet_t packet_header_t;

#define MAX_BUF_SIZE	1024// һ�������շ���󻺴��С(�Զ��������С�������ܳ��� 65535 + sizeof(packet_header_t))
#define MAX_PACKETSZ   ((MAX_BUF_SIZE - sizeof(packet_header_t)) >= 65535 ? 65535:(MAX_BUF_SIZE - sizeof(packet_header_t)) ) // �����ݰ������С������65535

// ��ȡ���ݰ���ʽ/��ʼ��/������/�ܰ���/�����С/β�������С
#define __packet_get_type(flag)		(u8_t )(((flag) & 0x0000000000000003)      )
#define __packet_get_start(flag)	(u8_t )(((flag) & 0x0000000000000004) >> 2 )
#define	__packet_get_end(flag)		(u8_t )(((flag) & 0x0000000000000008) >> 3 )
#define __packet_get_total(flag)	(u32_t)(((flag) & 0x00000000FFFFFFF0) >> 4 )
#define __packet_get_len(flag)		(u16_t)(((flag) & 0x0000FFFF00000000) >> 32)
#define __packet_get_tail(flag)     (u16_t)(((flag) & 0xFFFF000000000000) >> 48)

// ָ�����ݰ���ʽ/��ʼ��/������/�ܰ���/�����С/β�������С
#define __packet_set_type( flag, t)	((flag) = ( ((flag) & 0xFFFFFFFFFFFFFFFC) | (( (u8_t )(t))        & 0x0000000000000003) ))
#define __packet_set_start(flag, b)	((flag) = ( ((flag) & 0xFFFFFFFFFFFFFFFB) | ((((u8_t )(b)) <<  2) & 0x0000000000000004) ))
#define	__packet_set_end(  flag, b)	((flag) = ( ((flag) & 0xFFFFFFFFFFFFFFF7) | ((((u8_t )(b)) <<  3) & 0x0000000000000008) ))
#define __packet_set_total(flag, n)	((flag) = ( ((flag) & 0xFFFFFFFF0000000F) | ((((u32_t)(n)) <<  4) & 0x00000000FFFFFFF0) ))
#define __packet_set_len(  flag, l)	((flag) = ( ((flag) & 0xFFFF0000FFFFFFFF) | ((((u64_t)(l)) << 32) & 0x0000FFFF00000000) ))
#define __packet_set_tail( flag, l)	((flag) = ( ((flag) & 0x0000FFFFFFFFFFFF) | ((((u64_t)(l)) << 48) & 0xFFFF000000000000) ))

#define __packet_data_addr(p) (char *)((char *)(p) + sizeof(packet_header_t))

typedef xBuffer_p packet_pool_t;
typedef std::map<u64_t,std::pair<char*,u32_t>> map_packet_t;
typedef map_packet_t::iterator				  itor_packet_t;

// ******************************************* Ӧ�ò㿪�ŵ��ýӿ� *******************************************

/*
**
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
**
*/

#endif // _XPACKjoint_packetET_H_INCLUDE_
