#ifndef JSF_SOCKET_H_
#define JSF_SOCKET_H_

#include "jsf_type_defs.h"

#ifdef WIN32

#include <winsock2.h>
typedef SOCKET				RawSocket;
typedef Sint32				JSFSockLen;
typedef SOCKET				JSFSockPollFd;
struct no_support_epoll_event
{
	Sint32 no_support;
};
typedef struct no_support_epoll_event JSFSOCKEPollEvent;
#define JSFSOCK_SD_RECEIVE	SD_RECEIVE      /*!<< shutdown the reading side */
#define JSFSOCK_SD_SEND		SD_SEND         /*!<< shutdown the writing side */
#define JSFSOCK_SD_BOTH		SD_BOTH         /*!<< shutdown both sides */
#define JSF_MSG_NOSIGNAL	MSG_DONTROUTE
#define JSFSOCK_LAST_ERROR	WSAGetLastError()
#define JSF_TCP_NODELAY		TCP_NODELAY

#elif __LINUX__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
typedef Sint32				RawSocket;
typedef socklen_t			JSFSockLen;
typedef Sint32				JSFSockPollFd;
typedef struct epoll_event	JSFSOCKEPollEvent;
#define JSFSOCK_SD_RECEIVE	SHUT_RD    /*!<< shutdown the reading side */
#define JSFSOCK_SD_SEND		SHUT_WR    /*!<< shutdown the writing side */
#define JSFSOCK_SD_BOTH		SHUT_RDWR  /*!<< shutdown both sides */
#define JSF_MSG_NOSIGNAL	MSG_NOSIGNAL
#define JSFSOCK_LAST_ERROR	errno
#define JSF_TCP_NODELAY		TCP_NODELAY
#endif

typedef struct sockaddr		JSFSockAddr;
typedef struct sockaddr_in	JSFSockAddr_In;
typedef struct hostent		JSFSockHostent;
typedef struct in_addr		JSFSockInAddr;
typedef fd_set				JSFSockFdSet;
typedef SOCKET				JSFSockPollFd;
typedef struct timeval		JSFSockTimeVal;

#define JSFSOCK_AF_INET		AF_INET
#define JSFSOCK_INADDR_ANY	INADDR_ANY

#define	JSFSOCK_STREAM		SOCK_STREAM
#define	JSFSOCK_DGRAM		SOCK_DGRAM
#define	JSFSOCK_RAW			SOCK_RAW

#define JSFSOCK_SOCKET_ERROR	(-1)
#define JSFSOCK_INVALID_SOCKET	(-1)
#define JSFSOCK_EWOULDBLOCK		(EWOULDBLOCK)
#define JSFSOCK_SOMAXCONN		(4)
#define JSFSOCK_OPT_LEVEL		SOL_SOCKET

#define JSFSOCK_POLLIN			(1)
#define JSFSOCK_POLLOUT			(4)
#define JSFSOCK_POLLERR			(8)

#define JSFSOCK_FD_ZERO			FD_ZERO
#define JSFSOCK_FD_SET			FD_SET
#define JSFSOCK_FD_ISSET		FD_ISSET

#define JSFSOCK_STATUS_NULL					(0)
#define JSFSOCK_STATUS_SYSTEM_INIT			(2)
#define JSFSOCK_STATUS_TCP_MODE				(4)
#define JSFSOCK_STATUS_UDP_MODE				(8)
#define JSFSOCK_STATUS_LISTEN_INIT			(16)
#define JSFSOCK_STATUS_UDP_CONNECT_INIT		(32)
#define JSFSOCK_STATUS_CLOSE_REGIST			(64)
#define JSFSOCK_STATUS_SOCK_ENABLE			(128)
#define JSFSOCK_STATUS_SEND_CLOSE_REGIST	(256)

enum SocketConnectionMode
{
	SOCK_MODE_TCP = 0,
	SOCK_MODE_UDP,
};

class JSFSocket
{
	RawSocket mSocket;
	SocketConnectionMode mConnectionMode;
	Sint32 mRevents;
	Sint32 mRevent;
	Sint8 mIP[64];
	Uint16 mPort;
	Uint32 mStatus;
public:
	JSFSocket();
	virtual ~JSFSocket();
	JSFResult InitSocket(SocketConnectionMode How);
	JSFResult Bind(const Uint16 BindPort);
	JSFResult Close();
	JSFResult Listen();
	JSFResult Accept(JSFSocket* pConnectSock);
	JSFResult Connect(const char* iServerName, const Uint16 ServerPort, Sint32 Timeout);
	Sint32 Send(const void* pBuf, Sint32 Length, JSFResult& ret);
	Sint32 Recv(void* pBuf, Sint32 Length, JSFResult& ret);
	Sint32 SendTo(const void* pBuf, Sint32 Length, const JSFSockAddr* toAddr, Sint32 toLength, JSFResult& ret);
	Sint32 RecvFrom(void* pBuf, Sint32 Length, JSFSockAddr* fromAddr, Sint32* fromLength, JSFResult& ret);
	JSFResult SetSocketNonBlock(bool Flag);
	JSFResult Select(Uint64 Outmsec);
	char* InetNtoA(JSFSockAddr_In AddrIn);
	JSFSockHostent* GetHostByName(const char* iHostName);
	JSFSockHostent* GetHostByAddr(const char* ciAddr, Sint32 AddrLength);
	Sint32 GetPeerName(JSFSockAddr* pAddr);
	SocketConnectionMode GetConnectionMode();
	JSFSockAddr_In mAddrIn;
	Sint8* GetRemoteIP();
	Uint16 GetRemotePort();
	void SetRemoteIPAndPort();
	Sint32 SetSocketOption(int optLevel, int optName, char* optVal, int optLen);
	RawSocket& GetRawSocket();
	void SetStatusBit(bool enabled, Uint32 statusBit);
	bool CheckStatusBit(Uint32 statusBit);
private:
	JSFResult SetSocketNonBlock_Platform_Targeted(bool Flag);
};

#endif