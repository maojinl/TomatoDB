#include <stdio.h>
#include "jsf_socket_wrapper.h"
#include "jsf_client_property.h"
#include "jsf_server.h"

JSFSocketWrapper::JSFSocketWrapper()
	:mConnectionID(0)
{
}

JSFSocketWrapper::~JSFSocketWrapper()
{
}

JSFResult JSFSocketWrapper::InitializeBuffer(size_t writeBufSize, size_t readBufSize)
{
	JSFResult ret = JSFRESULT_OK;
	bool socketInit = mSocketSendBuffer.Initialize(writeBufSize);
	socketInit &= mSocketRecvBuffer.Initialize(readBufSize);
	if (!socketInit) 
	{
		ret = JSFRESULT_SOCKEX_ERR_MEMORY_FULL;
	}
	return ret;
}

JSFResult JSFSocketWrapper::InitSocket(Sint32 how)
{
	JSFResult ret = JSFRESULT_OK;
	ret = mSocket.InitSocket((SocketConnectionMode)how);
	return ret;
}

JSFResult JSFSocketWrapper::Close()
{
	JSFResult ret = JSFRESULT_OK;
	ret = mSocket.Close();
	return  ret;
}

JSFResult JSFSocketWrapper::SetupListenSocket(const Uint16 cListenPort)
{
	JSFResult ret = JSFRESULT_OK;
	ret = mSocket.Bind(cListenPort);
	ret = mSocket.Listen();
	return ret;
}

JSFResult JSFSocketWrapper::Accept(const Uint16 cListenPort, JSFSocketWrapper* pConnectSockWrapper)
{
	JSFResult ret = JSFRESULT_OK;
	ret = mSocket.Accept(&pConnectSockWrapper->mSocket);
	if (JSFRESULT_FAILED(ret)) {
		return ret;
	}
	return ret;
}

JSFResult JSFSocketWrapper::Connect(const char* pServerName, const Uint16 cServerPost, Sint32 Timeout)
{
	JSFResult ret = JSFRESULT_OK;
	ret = mSocket.Connect(pServerName, cServerPost, Timeout);
	return ret;
}

JSFResult JSFSocketWrapper::WriteMsg(const char* iBuf, Sint32 length)
{
	JSFResult ret = JSFRESULT_OK;
	size_t headSpace = 0;
	size_t tailSpace = 0;
	size_t freeBufSize = 0;

	JSFAutoMutexLocker autoLocker(mMutexSendBufferWrite);

	freeBufSize = mSocketSendBuffer.GetBufferFreeSpaces(headSpace, tailSpace);

	if (freeBufSize <= 0 || length >= freeBufSize)
	{
		printf("[sockex2] buffer limit check\n");
		ret = JSFRESULT_SOCKEX_WRN_WRITE_BUFFER;
		return ret;
	}
	
	if (headSpace == 0 || length < tailSpace)
	{
		memcpy(mSocketSendBuffer.GetWritePosition(), iBuf, length);
		mSocketSendBuffer.AddWritePoint(length);
	}
	else
	{
		memcpy(mSocketSendBuffer.GetWritePosition(), iBuf, tailSpace);
		mSocketSendBuffer.AddWritePoint(tailSpace);
		size_t left = length - tailSpace;
		memcpy(mSocketSendBuffer.GetStartPosition(), &iBuf[tailSpace], left);
		mSocketSendBuffer.AddWritePoint(left);
	}
	return ret;
}

JSFResult JSFSocketWrapper::Flush()
{
	JSFResult ret = JSFRESULT_OK;
	size_t sentByte;
	size_t sentByte2;
	size_t headSpace = 0;
	size_t tailSpace = 0;
	size_t filledBufSize = 0;
	size_t left;

	JSFAutoMutexLocker autoLocker(mMutexSendBufferWrite);

	filledBufSize = mSocketSendBuffer.GetBufferFilledSpaces(headSpace, tailSpace);

	if (filledBufSize == 0)
	{
		return ret;
	}

	JSFResult sockResult;
	if (headSpace == 0)
	{
		sentByte = SendCore(mSocketSendBuffer.GetReadPosition(), filledBufSize, sockResult);
		if (sentByte > 0)
		{
			mSocketSendBuffer.AddReadPoint(sentByte);
		}
	}
	else
	{
		sentByte = SendCore(mSocketSendBuffer.GetReadPosition(), tailSpace, sockResult);
		if (sentByte > 0)
		{
			mSocketSendBuffer.AddReadPoint(sentByte);
			if (sentByte == tailSpace)
			{
				left = filledBufSize - tailSpace;
				sentByte2 = SendCore(mSocketSendBuffer.GetReadPosition(), left, sockResult);
				if (sentByte2 > 0)
				{
					mSocketSendBuffer.AddReadPoint(sentByte2);
					sentByte += sentByte2;
				}
			}
		}
	}

	if (sockResult == JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK)
	{
		printf("[SocketWrapper Flush] JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK  sent bytes %ld\n", sentByte);
		ret = JSFRESULT_SOCK_WRN_SEND_EWOULDBLOCK;
	}
	else if (JSFRESULT_FAILED(sockResult))
	{
		printf("[SocketWrapper Flush] send error socket close\n");
		ret = JSFRESULT_SOCKEX_ERR_WRITE;
	}

	return ret;
}

Sint32 JSFSocketWrapper::SendCore(const void* pBuf, Sint32 length, JSFResult& ret)
{
	if (mSocket.GetConnectionMode() == SOCK_MODE_TCP)
	{
		return mSocket.Send(pBuf, length, ret);
	}
	else if (mSocket.GetConnectionMode() == SOCK_MODE_UDP)
	{
		return mSocket.SendTo(pBuf, length, NULL, 0, ret);
	}
	printf("[SocketWrapper SendCore] wrong connection mode.\n");
	return -1;
}

JSFResult JSFSocketWrapper::Read()
{
	JSFResult ret = JSFRESULT_OK;
	Sint32 readByte = 0;
	size_t headSpace = 0;
	size_t tailSpace = 0;
	size_t freeBufSize = 0;
	Sint32 addrSize;

	freeBufSize = mSocketRecvBuffer.GetBufferFreeSpaces(headSpace, tailSpace);
	
	if (freeBufSize <= 0)
	{
		printf("[sockex2] buffer limit check\n");
		ret = JSFRESULT_SOCKEX_WRN_READ_BUFFER;
	}

	JSFResult sockResult;
	if (headSpace == 0)
	{
		readByte = RecvCore(mSocketRecvBuffer.GetWritePosition(), tailSpace, sockResult);
		if (readByte >= 0)
		{
			mSocketRecvBuffer.AddReadPoint(readByte);
		}
	}
	else
	{
		readByte = RecvCore(mSocketRecvBuffer.GetWritePosition(), tailSpace, sockResult);
		if (readByte >= 0)
		{
			mSocketSendBuffer.AddWritePoint(readByte);
			if (readByte == tailSpace)
			{
				size_t left = readByte - tailSpace;
				size_t readByte2 = RecvCore(mSocketRecvBuffer.GetWritePosition(), left, sockResult);
				if (readByte2 >= 0)
				{
					mSocketSendBuffer.AddWritePoint(readByte2);
					readByte += readByte2;
				}
			}
		}
	}

	if (JSFRESULT_FAILED(sockResult))
	{
		ret = mpUserData->GetOwner()->CallBackSocketEvent(mpUserData, JSFSOCKET_MSG_READ, &mSocketRecvBuffer);
		ret = JSFRESULT_SOCKEX_ERR_READ;
	}
	return ret;
}

Sint32 JSFSocketWrapper::RecvCore(void* pBuf, Sint32 length, JSFResult& ret)
{
	if (mSocket.GetConnectionMode() == SOCK_MODE_TCP)
	{
		return mSocket.Recv(pBuf, length, ret);
	}
	else if (mSocket.GetConnectionMode() == SOCK_MODE_UDP)
	{
		Sint32 addrSize = sizeof(JSFSockAddr_In);
		return mSocket.RecvFrom(pBuf, length, (JSFSockAddr*)&mSocket.mAddrIn, &addrSize, ret);
	}
	printf("[SocketWrapper RecvCore] wrong connection mode.\n");
	return -1;
}

JSFResult JSFSocketWrapper::SetNonBlock()
{
	return mSocket.SetSocketNonBlock(1);
}

void JSFSocketWrapper::SetUserData(JSFClientProperty* pUserData)
{
	mpUserData = pUserData;
	return;
}

Sint8* JSFSocketWrapper::GetRemoteIP()
{
	return mSocket.GetRemoteIP();
}

Uint16 JSFSocketWrapper::GetRemotePort()
{
	return mSocket.GetRemotePort();
}

void JSFSocketWrapper::SetRemoteIPAndPort()
{
	mSocket.SetRemoteIPAndPort();
	return;
}

Sint32 JSFSocketWrapper::SetSocketOption(int optLevel, int optName, char* optVal, int optLen)
{
	return mSocket.SetSocketOption(optLevel, optName, optVal, optLen);
}
