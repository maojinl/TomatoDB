#ifndef JSF_SERVER_SOCKETS_MGR_H_
#define JSF_SERVER_SOCKETS_MGR_H_

#include <list>
#include "jsf_socket.h"
#include "jsf_socket_ring_buffer.h"
#include "jsf_thread.h"
#include "jsf_socket_wrapper.h"
#include "jsf_memory_pool.h"
#include "jsf_thread.h"

using namespace std;

class JSFServer;

typedef enum
{
	JSFSOCKET_MSG_WRITE = 1,
	JSFSOCKET_MSG_READ,
	JSFSOCKET_MSG_SOCKET_CLOSE,
	JSFSOCKET_MSG_SEND_EWOULDBLOCK,
	JSFSOCKET_MSG_USER,
	JSFSOCKET_MSG_FETCH,
} JSFSocketEventMsg;

typedef enum
{
	JSFSOCKET_MSG_ACCEPT = 1,
	JSFSOCKET_MSG_ACCEPT_ERR,
	JSFSOCKET_MSG_ACCEPT_ERR_MEMORY_EMPTY,
	JSFSOCKET_MSG_ACCEPT_ERR_CONNECT_MAX,
} JSFSocketAcceptMsg;

class JSFServerSocketsMgr
{
	typedef list<JSFSocketWrapper*> SocketWrappersList;
public:
	JSFServerSocketsMgr();
	virtual ~JSFServerSocketsMgr();
	JSFResult Init(Sint32 how, JSFServer* pOwner, Sint32 connectNum, Sint32 readBufSize, Sint32 writeBufSize);
	bool mIsAcceptRunning;
	Sint32 mConnNowNum;
	Uint32 Acceptor();
	static JSFThread_CallBackResult JSFServerSocketsMgr_Acceptor(void* pThread);
	JSFResult SetupListenerAndAcceptor(const Uint16 cListenPort, Sint32 SockOpt, Sint32 listenerID);
	JSFResult ReadAllEvent(Sint32 TimeOut);
	JSFResult FlushAllEvent();
private:
	Sint32 mRunning;
	Sint32 mListenRun;
	JSFThread* mpAcceptorThread;
	Uint16 mListenPort;
	JSFSocketWrapper* mpListenSocket;
	Sint32 mSockOpt;
	SocketWrappersList mSockList;
	JSFThreadLock mSockListMutex;
	Sint32 mSockGUID;
	list<JSFSockPollFd*> mlPollFdList;
	Sint32 mConnMax;
	Sint32 mUseEPoll;
	JSFSockPollFd mEPollFd;
	list<JSFSOCKEPollEvent*> mpEPollEvents;

	JSFMemoryPool<JSFSocketWrapper> mSocketWrapperMemPool;

	Sint32 mSockListCount;

	Sint64 mReserve0;
	Sint64 mReserve1;
	Sint64 mReserve2;
	Sint64 mReserve3;

	JSFServer* mpOwner;
	JSFResult SocketsPoll_Platform_Targeted(Sint32& retSelect, Sint32 TimeOut);
};

#endif