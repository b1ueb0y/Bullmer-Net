/*
* @note		: �û�������
* @author	��Andy.Ro
* @email	��Qwuloo@qq.com
* @date		��2014-06
* @mark		���õ����ĸ��ࣺxClient��xClientFactory��xConnection��xConnectionFactory ���ⲿ�̳�(λ��/xNet/Ŀ¼��)
*
*/

#include "../xNet/xNet.h"
#include "../xNet/xFactory.h"
#include "../xNet/xClient.h"
#include "../xNet/xConnection.h"

#include "../xSys/xSysMsg.h"
#include "../xSys/xSys_db.h"
#include "UserServer.h"

//#include "../xNet/xNetDll.h"
#include "../xNet/xReactor.h"

// ���ڷ����û��ռ��ڴ��
// typedef xBuffer<C2SClient> client_pool_t;
// static client_pool_t g_client_pool(10,0);

static u32_t g_sID = 0; // ϵͳ��ID
static u32_t g_pID = 0; // ƽ̨��ID
static C2S_Server::map_uID_C2S_client g_sID_S2P_client;

//------------------------------------------------------------------------
// C2SClient
//------------------------------------------------------------------------
C2SClient::C2SClient(C2S_Server* factory):xClient(factory),_uid(0)
{

}

C2SClient* C2SClient::init(C2S_Server* factory)
{
	this->_factory = factory;
	this->_uid = 0;
	return this;
}

int C2SClient::onMade(void)
{
	s8_t l_ipaddr[128],r_ipaddr[128];
	this->peer_ipaddr(r_ipaddr, sizeof(r_ipaddr));
	this->self_ipaddr(l_ipaddr, sizeof(l_ipaddr));
	
	printf("[C2SClient] new connect: %s:%d,listening: %s:%d total:%d\n",r_ipaddr,this->port(),l_ipaddr,this->port(),_factory->count());
	
	return 0;
}

int C2SClient::onLost(u32_t err)
{
	s8_t l_ipaddr[128],r_ipaddr[128];
	this->peer_ipaddr(r_ipaddr, sizeof(r_ipaddr));
	this->self_ipaddr(l_ipaddr, sizeof(l_ipaddr));

	printf("[C2SClient] lost connect: %s:%d,listening: %s:%d total:%d\n",r_ipaddr,this->port(),l_ipaddr,this->port(),_factory->count());

	return __super::onLost(err);
}

int C2SClient::onRecv(char const* buf, u64_t len)
{
	s8_t l_ipaddr[128],r_ipaddr[128];
	this->peer_ipaddr(r_ipaddr, sizeof(r_ipaddr));
	this->self_ipaddr(l_ipaddr, sizeof(l_ipaddr));

	// �������������������������

	dynamic_cast<C2S_Server*>(this->_factory)->onMsg(this,buf,len);
	
	return 0;
}

C2SClient::~C2SClient(void)
{
} // class C2SClient

//------------------------------------------------------------------------
// C2S_Server
//------------------------------------------------------------------------
C2S_Server::C2S_Server(void):m_sid(1),m_pid(1)
{

}

xClient * C2S_Server::create(void)
{
	// �����ֽڷ��䷽ʽ����֧���麯�����ã�������ʵ��֧���麯�����ڴ��
	//	return g_user_pool.alloc()->get_data()->init(this);
	return new C2SClient(this);
}

void C2S_Server::free(xClient * object)
{
	object->closesocket();

	xReactor::dispatcher()->dispatch(object, EVENT_CLOSE_DESTORY);
}

void C2S_Server::onAdd(xClient * object)
{

}

void C2S_Server::onDel(xClient * object)
{

}

void C2S_Server::onMsg(xClient * object, char const* buf, u64_t len)
{
	MSG_Root * pmsg = (MSG_Root *)buf;
	switch ( pmsg->id() )
	{
	case ID_C2S_REGISTER_Request:
		{
			C2S_REGISTER_Request * req = (C2S_REGISTER_Request *)pmsg;

			xSys_db * db = alloc_database_object();
			MYSQL_ROW result = db->proc_register(req->username,
												 req->passwd,
												 req->nickname,
												 req->sex);
			free_database_object(db);
			
			if ( db->is_empty(result) )
			{
				printf("database error..\n");
				return ;
			}
			switch ( atoi(result[0]) )
			{
			case  1:
				{
					S2C_REGISTER_Response rsp;
					rsp.code = atoi(result[0]);
					memcpy_s(rsp.errmsg, strlen(result[1]),result[1],strlen(result[1]));
					memcpy_s(rsp.username, strlen(result[2]),result[2],strlen(result[2]));
					memcpy_s(rsp.passwd, strlen(result[3]),result[3],strlen(result[3]));
					object->_transport.send((char const *)&rsp, sizeof(S2C_REGISTER_Response));
					printf("{ code: %d, errmsg: \"%s\" }\n",rsp.code,rsp.errmsg);
					break;
				}

			default:
				{
					S2C_REGISTER_Response rsp;
					rsp.code = atoi(result[0]);
					memcpy_s(rsp.errmsg, strlen(result[1]),result[1],strlen(result[1]));
					object->_transport.send((char const *)&rsp, sizeof(S2C_REGISTER_Response));
					printf("{ code: %d, errmsg: \"%s\" }\n",rsp.code,rsp.errmsg);
					break;
				}
			}
			break;
		}
	case ID_C2S_LOGIN_Request:
		{
			C2S_LOGIN_Request * req = (C2S_LOGIN_Request *)pmsg;
			
			xSys_db * db = alloc_database_object();
			MYSQL_ROW result = db->proc_login(req->username,
											  req->passwd,
											  req->state,
											  req->sID,
											  req->pID,
											  req->ipaddr,
											  req->geo[LNG],
											  req->geo[LAT]);
			free_database_object(db);
			
			if ( db->is_empty(result) )
			{
				printf("database error..\n");
				return ;
			}
			switch ( atoi(result[0]) )
			{
			case  1:
				{
					S2C_LOGIN_Response rsp;
					rsp.code = atoi(result[0]);
					memcpy_s(rsp.errmsg, strlen(result[1]),result[1],strlen(result[1]));
					rsp.userid = atoi(result[2]);
					rsp.sessionid = atoi(result[3]);
					object->_transport.send((char const *)&rsp, sizeof(S2C_LOGIN_Response));
					printf("{ code: %d, errmsg: \"%s\",uid: %d, sessid: %d }\n",rsp.code,rsp.errmsg,rsp.userid,rsp.sessionid);
				}

			default:
				{
					
				}
			}
			break;
		}
	default:
		{
			char buf_[200] = {0};
			memcpy(buf_, buf, (u32_t)len);
			printf("[C2SClient(fd:%d)]��%s\n",object->_object.fd, buf_);

			char tbuf[][200]=
			{
				"1.��ã�����~~~~~~~~~",
				"2.hello,world !",
				"3.�㵽��������أ��Ҷ����� ��",
				"4.лл���������ң������������ˣ�Ҫ�ĵİ���",
				"5.һ��Ҫ�ǵ�֪ͨ�Ұ�������Ȼ���ٺ�~",
				"6.������",
				"7.�ǵ���!!!",
				"8.�ķ�����!!!",
				"9.���Ž�ͳһ!!!",
				"10.�����ݱ��Ȼ��ͷ!!!",
				"11.�о��������ط���������Ϳѻ!!!",
			};
			int idx = rand()%(sizeof(tbuf)/sizeof(tbuf[0]));
			if( NO_ERROR != object->_transport.send(tbuf[idx],strlen(tbuf[idx])) )
			{

			}
			break;
		}
	}
}

C2S_Server::~C2S_Server(void)
{

} // class C2S_Server

//------------------------------------------------------------------------
// S2PConnection
//------------------------------------------------------------------------
S2PConnection::S2PConnection(S2P_Server* factory):xConnection(factory)
{

}

S2PConnection* S2PConnection::init(S2P_Server* factory)
{
	this->_factory = factory;
	return this;
}

int S2PConnection::onMade(void)
{
	s8_t l_ipaddr[128],r_ipaddr[129];
	ipaddr2str(this->_object.l_ipaddr,l_ipaddr,sizeof(l_ipaddr));
	ipaddr2str(this->_object.r_ipaddr,r_ipaddr,sizeof(r_ipaddr));
	printf("[S2PConnection] connect to: %s:%d total:%d\n",r_ipaddr,this->_object.port,_factory->count());

	printf("���ڵǼ�ע�ᵽƽ̨��...\n");
	if ( NO_ERROR != this->S2PRegister() )
	{
		printf("�Ǽ�ע��ʧ��.\n",errno);
	}

	return 0;
}

int S2PConnection::onLost(u32_t err)
{
	s8_t l_ipaddr[128],r_ipaddr[129];
	ipaddr2str(this->_object.l_ipaddr,l_ipaddr,sizeof(l_ipaddr));
	ipaddr2str(this->_object.r_ipaddr,r_ipaddr,sizeof(r_ipaddr));
	printf("[S2PConnection] lost connect to: %s:%d total:%d\n",r_ipaddr,this->_object.port,_factory->count());

	return __super::onLost(err);
}

int S2PConnection::onRecv(char const* buf, u64_t len)
{
	s8_t l_ipaddr[128],r_ipaddr[129];
	ipaddr2str(this->_object.l_ipaddr,l_ipaddr,sizeof(l_ipaddr));
	ipaddr2str(this->_object.r_ipaddr,r_ipaddr,sizeof(r_ipaddr));

	// �������������������������
	dynamic_cast<S2P_Server*>(this->_factory)->onMsg(this,buf,len);
	return 0;
}

int S2PConnection::S2PRegister(void)
{
	char hname[50],ipaddr[50];
	localhnameipaddr(hname,ipaddr,50);

	S2P_REGISTER_Request msg;
	memcpy_s(msg.sname, strlen(hname), hname, strlen(hname));
	msg.ipaddr = str2ipaddr(ipaddr);
	msg.umax = 5000;

	return this->send((char const *)&msg, sizeof(msg));
}

S2PConnection::~S2PConnection(void)
{
} // class S2PConnection

//------------------------------------------------------------------------
// S2P_Server
//------------------------------------------------------------------------
S2P_Server::S2P_Server(void)
{

}

xConnection * S2P_Server::create(void)
{
	// �����ֽڷ��䷽ʽ����֧���麯�����ã�������ʵ��֧���麯�����ڴ��
	//	return g_user_pool.alloc()->get_data()->init(this);
	return new S2PConnection(this);
}

void S2P_Server::free(xConnection * object)
{
	object->closesocket();

	xReactor::dispatcher()->dispatch(object, EVENT_CLOSE_DESTORY);
}

void S2P_Server::onAdd(xConnection * object)
{

}

void S2P_Server::onDel(xConnection * object)
{

}

void S2P_Server::onMsg(xConnection * object, char const* buf, u64_t len)
{
	MSG_Root * pmsg = (MSG_Root *)buf;
	switch ( pmsg->id() )
	{
	case ID_P2S_REGISTER_Response:
		{
			P2S_REGISTER_Response * rsp = (P2S_REGISTER_Response *)pmsg;
			g_sID = rsp->sID;
			g_pID = rsp->pID;
			printf("{ code: %d, errmsg: \"%ssID:%dpID:%d\" }\n",rsp->code, rsp->errmsg, rsp->sID, rsp->pID);

			char tbuf[][200]=
			{
				"1.��ã�����~~~~~~~~~",
				"2.hello,world !",
				"3.�㵽��������أ��Ҷ����� ��",
				"4.лл���������ң������������ˣ�Ҫ�ĵİ���",
				"5.һ��Ҫ�ǵ�֪ͨ�Ұ�������Ȼ���ٺ�~",
				"6.������",
				"7.�ǵ���!!!",
				"8.�ķ�����!!!",
				"9.���Ž�ͳһ!!!",
				"10.�����ݱ��Ȼ��ͷ!!!",
				"11.�о��������ط���������Ϳѻ!!!",
			};
			int idx = rand()%(sizeof(tbuf)/sizeof(tbuf[0]));
			if( NO_ERROR != object->_transport.send(tbuf[idx],strlen(tbuf[idx])) )
			{

			}

			break;
		}
	default:
		{
			char buf_[200] = {0};
			memcpy(buf_, buf, (u32_t)len);
			printf("[S2PConnection(fd:%d)]��%s\n",object->_object.fd, buf_);

			char tbuf[][200]=
			{
				"1.��ã�����~~~~~~~~~",
				"2.hello,world !",
				"3.�㵽��������أ��Ҷ����� ��",
				"4.лл���������ң������������ˣ�Ҫ�ĵİ���",
				"5.һ��Ҫ�ǵ�֪ͨ�Ұ�������Ȼ���ٺ�~",
				"6.������",
				"7.�ǵ���!!!",
				"8.�ķ�����!!!",
				"9.���Ž�ͳһ!!!",
				"10.�����ݱ��Ȼ��ͷ!!!",
				"11.�о��������ط���������Ϳѻ!!!",
			};
			int idx = rand()%(sizeof(tbuf)/sizeof(tbuf[0]));
			if( NO_ERROR != object->_transport.send(tbuf[idx],strlen(tbuf[idx])) )
			{

			}
			break;
		}
	}
}

S2P_Server::~S2P_Server(void)
{

} // class S2P_Server