#ifndef JSF_SOCKET_WRAPPER_H_
#define JSF_SOCKET_WRAPPER_H_

#include "jsf_socket.h"
#include "jsf_socket_ring_buffer.h"
#include "jsf_thread_lock.h"

class JSFClientProperty;

class JSFSocketWrapper
{
protected:
	JSFSocket mSocket;
	Sint32 mConnectID;
	Sint32 mOwnerID;

	JSFSocketRingBuffer mSocketSendBuffer;
	JSFSocketRingBuffer mSocketRecvBuffer;
	JSFClientProperty* mpUserData;

	JSFThreadLock mMutexSendBufferWrite;

	Sint32 mReserve0;
	Sint32 mReserve1;
	Sint32 mReserve2;
	Sint32 mReserve3;
	Sint32 SendCore(const void* pBuf, Sint32 length, JSFResult& ret);
	Sint32 RecvCore(void* pBuf, Sint32 length, JSFResult& ret);
public:
	Sint32 mRevents;
	JSFSocketWrapper();
	virtual ~JSFSocketWrapper();
	JSFResult InitializeBuffer(size_t WriteBufSize, size_t ReadBufSize);
	JSFResult InitSocket(Sint32 How);
	JSFResult Close();
	JSFResult SetupListenSocket(const Uint16 cListenPort);
	JSFResult Accept(const Uint16 cListenPort, JSFSocketWrapper* pConnectSockWrapper);
	JSFResult Connect(const char* pServerName, const Uint16 cServerPost, Sint32 Timeout);
	JSFResult WriteMsg(const char* iBuf, Sint32 length);
	JSFResult Flush();
	JSFResult Read();
	JSFResult SetNonBlock();
	Sint32 mConnectionID;
	void SetUserData(JSFClientProperty* pUserData);
	JSFClientProperty* GetUserData() { return mpUserData; }
	Sint8* GetRemoteIP();
	Uint16 GetRemotePort();
	void SetRemoteIPAndPort();
	Sint32 SetSocketOption(int optLevel, int optName, char* optVal, int optLen);
	void SetConnectID(Sint32 connectID) { mConnectID = connectID; return; }
	void SetOwnerID(Sint32 ownerID) { mOwnerID = ownerID; return; }
	Sint32 GetConnectID() { return mConnectID; }
	Sint32 GetOwnerID() { return mOwnerID; }
	JSFSocket& GetSocket() { return mSocket; }
	JSFSocketRingBuffer& GetRecvBuffer() { return mSocketRecvBuffer; }
};

#endif