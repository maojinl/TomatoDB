#include <stdio.h>
#include "jsf_socket.h"

JSFSocket::JSFSocket()
	:mStatus(JSFSOCK_STATUS_NULL)
{
}

JSFResult JSFSocket::InitSocket(SocketConnectionMode how)
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 option;
	switch (how) {
	case SOCK_MODE_TCP:
		mSocket = socket(JSFSOCK_AF_INET, JSFSOCK_STREAM, 0);
		if (mSocket == JSFSOCK_SOCKET_ERROR)
		{
			printf("Socket Error = %d", errno);
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
		else
		{
			option = 1;
			setsockopt(mSocket, JSFSOCK_OPT_LEVEL, SO_REUSEADDR, (Sint8*)&option, sizeof(option));
			mConnectionMode = how;
		}
		break;
	case SOCK_MODE_UDP:
		mSocket = socket(JSFSOCK_AF_INET, JSFSOCK_DGRAM, 0);
		if (mSocket == JSFSOCK_SOCKET_ERROR) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
		else {
			option = 1;
			setsockopt(mSocket, JSFSOCK_OPT_LEVEL, SO_REUSEADDR, (Sint8*)&option, sizeof(option));
			mConnectionMode = how;
		}
		break;

	default:
		ret = JSFRESULT_ERR_ARGUMENT;
		break;
	}
	return  ret;
}

JSFResult JSFSocket::Close()
{
	JSFResult ret = JSFRESULT_OK;
	if (mSocket < 0)
	{
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}
	shutdown(mSocket, JSFSOCK_SD_BOTH);
	closesocket(mSocket);
	return ret;
}

JSFResult JSFSocket::Listen()
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 vRet = JSFSOCK_SOCKET_ERROR;
	if (mSocket < 0) {
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}

	vRet = listen(mSocket, JSFSOCK_SOMAXCONN);
	if (vRet < 0) {
		printf("error : %d\n", JSFSOCK_LAST_ERROR);
		ret = JSFRESULT_SOCK_ERR_LISTEN;
	}
	SetStatusBit(true, JSFSOCK_STATUS_LISTEN_INIT);
	return ret;
}

JSFResult JSFSocket::Accept(JSFSocket* pConnectSock)
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 AddrInSize = 0;

	if (mSocket < 0) {
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}
	pConnectSock->mSocket = JSFSOCK_INVALID_SOCKET;
	AddrInSize = sizeof(pConnectSock->mAddrIn);
	pConnectSock->mSocket = accept(mSocket, (JSFSockAddr*)&(pConnectSock->mAddrIn), (JSFSockLen*)&AddrInSize);
	if (pConnectSock->mSocket < 0) {
		printf("iConnectSock->mSocket = %d\n", pConnectSock->mSocket);
		printf("AddrInSize           = %d\n", AddrInSize);
		printf("error : %d\n", JSFSOCK_LAST_ERROR);
		ret = JSFRESULT_SOCK_ERR_ACCEPT;
		return ret;
	}
	pConnectSock->mConnectionMode = mConnectionMode;
	return  ret;
}

JSFResult JSFSocket::SetSocketNonBlock(bool Flag)
{
	JSFResult ret = JSFRESULT_OK;

	if (mSocket < 0) {
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}
	SetSocketNonBlock_Platform_Targeted(Flag);
	return ret;
}

#ifdef __LINUX__
JSFResult JSFSocket::SetSocketNonBlock_Platform_Targeted(bool Flag)
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 CntFlag;

	if (Flag) {
		if ((CntFlag = fcntl(mSocket, F_GETFL, 0)) < 0) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
			return ret;
		}
		CntFlag |= O_NONBLOCK;
		if (fcntl(mSocket, F_SETFL, CntFlag) < 0) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
	}
	else {
		if ((CntFlag = fcntl(mSocket, F_GETFL, 0)) < 0) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
			return ret;
		}
		CntFlag &= ~O_NONBLOCK;
		if (fcntl(mSocket, F_SETFL, CntFlag) < 0) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
	}
	return ret;
}
/*LINUX*/

#elif WIN32

JSFResult JSFSocket::SetSocketNonBlock_Platform_Targeted(bool Flag)
{
	JSFResult ret = JSFRESULT_OK;
	ULong CntFlag;

	if (Flag) {
		CntFlag = 1L;
		if (ioctlsocket(mSocket, FIONBIO, &CntFlag) == JSFSOCK_SOCKET_ERROR) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
	}
	else {
		CntFlag = 0L;
		if (ioctlsocket(mSocket, FIONBIO, &CntFlag) == JSFSOCK_SOCKET_ERROR) {
			ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		}
	}
	return  ret;
}
#endif/*WIN*/

JSFResult JSFSocket::Connect(const char* iServerName, const Uint16 ServerPort, Sint32 Timeout)
{
	JSFResult ret = JSFRESULT_OK;
	JSFSockHostent*  pHost;
	Sint32 vRet;

	if (mSocket < 0)
	{
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}

	memset((Sint8*)&mAddrIn, 0, sizeof(mAddrIn));
	mAddrIn.sin_family = JSFSOCK_AF_INET;
	mAddrIn.sin_port = htons(ServerPort);
	mAddrIn.sin_addr.s_addr = inet_addr(iServerName);
	if (mAddrIn.sin_addr.s_addr == -1)
	{
		pHost = gethostbyname(iServerName);
		if (pHost != NULL)
		{
			if (pHost->h_addr == NULL)
			{
				ret = JSFRESULT_SOCK_ERR_CONNECT_HOSTNAME;
				return ret;
			}
			mAddrIn.sin_addr.s_addr = ((JSFSockInAddr*)pHost->h_addr)->s_addr;
		}
		else
		{
			ret = JSFRESULT_SOCK_ERR_CONNECT_HOSTNAME;
			return ret;
		}
	}

	if (mConnectionMode == SOCK_MODE_TCP)
	{
		if (Timeout > 0)
		{
			SetSocketNonBlock(true);
			vRet = connect(mSocket, (JSFSockAddr*)&(mAddrIn), sizeof(mAddrIn));
			if (vRet < 0)
			{
				Select(Timeout);
				if (mRevents & JSFSOCK_POLLIN)
				{
					ret = JSFRESULT_OK;
				}
				else
				{
					ret = JSFRESULT_SOCK_ERR_CONNECT;
				}
			}
			SetSocketNonBlock(true);
			return ret;
		}
		else
		{
			vRet = connect(mSocket, (JSFSockAddr*)&(mAddrIn), sizeof(mAddrIn));
			if (vRet < 0) 
			{
				ret = JSFRESULT_SOCK_ERR_CONNECT;
				printf("iConnectSock->mSocket = %d iServerName =%s, server port = %d\n", mSocket, iServerName, ServerPort);
				return ret;
			}
		}
	}
	else
	{
		mConnectionMode = SOCK_MODE_UDP;
		return ret;
	}
	return ret;
}

JSFResult JSFSocket::Select(Uint64 Outmsec)
{
	JSFResult Ret = JSFRESULT_OK;
	Sint32 vRet = JSFSOCK_SOCKET_ERROR;
	timeval Tv;
	JSFSockFdSet Rmask;
	JSFSockFdSet Wmask;
	JSFSockFdSet Emask;

	if (mSocket < 0) {
		Ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return Ret;
	}

	JSFSOCK_FD_ZERO(&Rmask);
	JSFSOCK_FD_ZERO(&Wmask);
	JSFSOCK_FD_ZERO(&Emask);
	JSFSOCK_FD_SET(mSocket, &Rmask);
	JSFSOCK_FD_SET(mSocket, &Wmask);
	JSFSOCK_FD_SET(mSocket, &Emask);

	Tv.tv_sec = (Uint32)Outmsec / 1000;
	Tv.tv_usec = (Uint32)(Outmsec % 1000);

	vRet = select(FD_SETSIZE, &Rmask, &Wmask, &Emask, &Tv);
	if (vRet < 0) {
		Ret = JSFRESULT_SOCK_ERR_SELECT;
		return Ret;
	}

	if (JSFSOCK_FD_ISSET(mSocket, &Rmask)) {
		mRevents |= JSFSOCK_POLLIN;
	}
	if (JSFSOCK_FD_ISSET(mSocket, &Wmask)) {
		mRevents |= JSFSOCK_POLLIN;
	}
	if (JSFSOCK_FD_ISSET(mSocket, &Emask)) {
		mRevents |= JSFSOCK_POLLERR;
	}
	return Ret;
}

JSFResult JSFSocket::Bind(const Uint16 BindPort)
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 RetCode = JSFSOCK_SOCKET_ERROR;

	if (mSocket < 0) {
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return ret;
	}

	mAddrIn.sin_family = JSFSOCK_AF_INET;
	mAddrIn.sin_port = htons(BindPort);
	mAddrIn.sin_addr.s_addr = JSFSOCK_INADDR_ANY;
	RetCode = bind(mSocket, (JSFSockAddr*)&mAddrIn, sizeof(mAddrIn));
	if (RetCode < 0)
	{
		ret = JSFRESULT_SOCK_ERR_BIND;
	}
	return  ret;
}

Sint32 JSFSocket::Send(const void* pBuf, Sint32 Length, JSFResult& ret)
{
	Sint32 sended_bytes = -1;
	ret = JSFRESULT_OK;

	if (mSocket < 0)
	{
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return sended_bytes;
	}

	sended_bytes = send(mSocket, (const char *)pBuf, Length, JSF_MSG_NOSIGNAL);
	if (sended_bytes < 0) {
		ret = (JSFResult)JSFSOCK_LAST_ERROR;
		if (ret == JSFSOCK_EWOULDBLOCK) {
			ret = JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK;
			return sended_bytes;
		}
		else {
#ifdef __LINUX__
			if (errno == EPIPE)
			{
				if (sended_bytes == 0)
				{
					ret = JSFRESULT_SOCK_ERR_SEND_CONNECT_CLOSE;
					return sended_bytes;
				}
			}
#endif
			ret = JSFRESULT_SOCK_ERR_SEND;
			return sended_bytes;
		}
	}
	else
	{
		return sended_bytes;
	}
}

Sint32 JSFSocket::Recv(void* pBuf, Sint32 Length, JSFResult& ret)
{
	Sint32 recved_bytes = -1;

	if (mSocket < 0)
	{
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return recved_bytes;
	}

	recved_bytes = recv(mSocket, (char *)pBuf, Length, JSF_MSG_NOSIGNAL);
	if (recved_bytes < 0)
	{
		ret = (JSFResult)JSFSOCK_LAST_ERROR;
		if (ret == JSFSOCK_EWOULDBLOCK)
		{
			ret = JSFRESULT_SOCK_WRN_RECV_EWOULDBLOCK;
			return recved_bytes;
		}
		else
		{
#ifdef __LINUX__
			if (errno == EPIPE)
			{
				if (recved_bytes == 0)
				{
					ret = JSFRESULT_SOCK_ERR_RECV_CONNECT_CLOSE;
					return recved_bytes;
				}
			}
#endif
			recved_bytes = JSFRESULT_SOCK_ERR_RECV;
			return recved_bytes;
		}
	}
	else
	{
		return recved_bytes;
	}
}

Sint32 JSFSocket::SendTo(const void* pBuf, Sint32 Length, const JSFSockAddr* toAddr, Sint32 toLength, JSFResult& ret)
{
	Sint32 sent_bytes = -1;

	if (mSocket < 0)
	{
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return sent_bytes;
	}

	if (toAddr != NULL)
	{
		sent_bytes = sendto(mSocket, (const char *)pBuf, Length, 0, (JSFSockAddr*)toAddr, toLength);
	}
	else if (mConnectionMode == SOCK_MODE_UDP)
	{
		sent_bytes = sendto(mSocket, (const char *)pBuf, Length, 0, (JSFSockAddr*)&mAddrIn, sizeof(JSFSockAddr));
	}
	else
	{
		ret = JSFRESULT_SOCK_ERR_CONNECT;
		return sent_bytes;
	}

	if (sent_bytes < 0)
	{
		ret = (JSFResult)JSFSOCK_LAST_ERROR;

		if (ret == JSFSOCK_EWOULDBLOCK)
		{
			ret = JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK;
			return sent_bytes;
		}
		else
		{
			ret = JSFRESULT_SOCK_ERR_SEND;
			return sent_bytes;
		}
	}
	else if (sent_bytes == 0)
	{
		ret = JSFRESULT_SOCK_ERR_SEND_CONNECT_CLOSE;
		return sent_bytes;
	}
	else
	{
		return sent_bytes;
	}
}

Sint32 JSFSocket::RecvFrom(void* pBuf, Sint32 Length, JSFSockAddr* fromAddr, Sint32* fromLength, JSFResult& ret)
{
	Sint32 recved_bytes = -1;

	if (mSocket < 0) {
		ret = JSFRESULT_SOCK_ERR_INVALIDSOCKET;
		return recved_bytes;
	}

	if (fromAddr != NULL)
	{
		recved_bytes = recvfrom(mSocket, (char *)pBuf, Length, 0, fromAddr, (JSFSockLen*)fromLength);
	}
	else
	{
		recved_bytes = recvfrom(mSocket, (char *)pBuf, Length, 0, (JSFSockAddr*)&mAddrIn, (JSFSockLen*)fromLength);
	}

	if (recved_bytes < 0)
	{
		ret = (JSFResult)JSFSOCK_LAST_ERROR;

		if (ret == JSFSOCK_EWOULDBLOCK)
		{
			ret = JSFRESULT_SOCK_WRN_RECV_EWOULDBLOCK;
			return recved_bytes;
		}
		else
		{
			ret = JSFRESULT_SOCK_ERR_RECV;
			return  recved_bytes;
		}
	}
	else if (recved_bytes == 0)
	{
		ret = JSFRESULT_SOCK_ERR_RECV_CONNECT_CLOSE;
		return recved_bytes;
	}
	else
	{
		return recved_bytes;
	}
}

char* JSFSocket::InetNtoA(JSFSockAddr_In AddrIn)
{
	return inet_ntoa(AddrIn.sin_addr);
}

JSFSockHostent* JSFSocket::GetHostByName(const char* ciHostName)
{
	return gethostbyname(ciHostName);
}

#pragma warning(suppress : 4996)
JSFSockHostent* JSFSocket::GetHostByAddr(const char* ciAddr, Sint32 AddrLength)
{
	return gethostbyaddr(ciAddr, AddrLength, JSFSOCK_AF_INET);
}

Sint32 JSFSocket::GetPeerName(JSFSockAddr* pAddr)
{
	Sint32 Length = sizeof(*pAddr);
	return getpeername(mSocket, pAddr, &Length);
}

SocketConnectionMode JSFSocket::GetConnectionMode()
{
	return mConnectionMode;
}

Sint8* JSFSocket::GetRemoteIP()
{
	return mIP;
}

Uint16 JSFSocket::GetRemotePort()
{
	return mPort;
}

void JSFSocket::SetRemoteIPAndPort()
{
	memset(mIP, 0, sizeof(mIP));
	strcpy(mIP, inet_ntoa(mAddrIn.sin_addr));
	mPort = ntohs(mAddrIn.sin_port);
}

Sint32 JSFSocket::SetSocketOption(int optLevel, int optName, char* optVal, int optLen)
{
	return setsockopt(mSocket, optLevel, optName, optVal, optLen);
}

void JSFSocket::SetStatusBit(bool enabled, Uint32 statusBit)
{
	if (enabled)
	{
		mStatus |= statusBit;
	}
	else
	{
		mStatus &= ~statusBit;
	}
}

bool JSFSocket::CheckStatusBit(Uint32 statusBit)
{
	bool ret = false;
	Uint32 i = mStatus & statusBit;
	if (i > 0)
	{
		ret = true;
	}
	return ret;
}

RawSocket& JSFSocket::GetRawSocket()
{
	return mSocket;
}

JSFSocket::~JSFSocket()
{
}
