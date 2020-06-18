#include <ctime>
#include "jsf_server.h"
#include "jsf_socket_ring_buffer.h"
#include "jsf_packet.h"
#include "jsf_socket_wrapper.h"
#include "jsf_client_property.h"
#include "jsf_onpacket_thread.h"

JSFServer::JSFServer()
{
}

JSFServer::~JSFServer()
{
	for (int i = 0; i < mOnPacketThreadsList.size(); i++)
	{
		delete mOnPacketThreadsList[i];
	}
	mOnPacketThreadsList.clear();
}

JSFResult JSFServer::OnSockMsgRead(JSFClientProperty* cProp, JSFSocketEventMsg Msg, JSFSocketRingBuffer* pRingBuffer)
{
	JSFResult ret = JSFRESULT_OK;
	cProp->SetOnReadTime();
	void *iBuf2 = NULL;
	Sint32 Length2 = 0;

	size_t headSpace = 0;
	size_t tailSpace = 0;
	size_t filledBufSize = 0;

	filledBufSize = pRingBuffer->GetBufferFilledSpaces(headSpace, tailSpace);

	if (filledBufSize == 0)
	{
		return ret;
	}

	JSFPacket* pPacket = new JSFPacket();
	if (headSpace > 0)
	{
		pPacket->FillPacket((char*)pRingBuffer->GetReadPosition(), headSpace);
		pRingBuffer->AddReadPoint(headSpace);
	}
	if (tailSpace > 0)
	{
		pPacket->FillPacket((char*)pRingBuffer->GetReadPosition(), tailSpace);
		pRingBuffer->AddReadPoint(tailSpace);
	}
	
	PostPacket(pPacket);

	return ret;
}

bool JSFServer::PostPacket(JSFPacket* pPacket)
{
	JSFAutoMutexLocker lock(mMutexPostPacket);
	JSFClientProperty *cprop = nullptr;
	std::srand(std::time(0));
	Sint32 randomIndex = std::rand() % mOnPacketThreadsList.size();
	
	JSFOnPacketThread* thread = mOnPacketThreadsList[randomIndex];
	if (thread->IsEmpty())
	{
		thread->PostPacket(pPacket);
		return true;
	}
	return true;
}


JSFResult JSFServer::CallBackSocketEvent(JSFClientProperty* cprop, JSFSocketEventMsg msg, JSFSocketRingBuffer* pRingBuffer)
{
	JSFResult ret = JSFRESULT_OK;

	switch (msg) {
	case JSFSOCKET_MSG_READ: return OnSockMsgRead(cprop, msg, pRingBuffer);
	/*case JSFSOCKET_MSG_WRITE: return  
	case JSFSOCKET_MSG_SOCKET_CLOSE: return  
	case JSFSOCKET_MSG_SEND_EWOULDBLOCK:  
	case JSFSOCKET_MSG_FETCH: return  
	case JSFSOCKET_MSG_USER: return  
	*/
	// for future implementation
	}
	
	return ret;
}

JSFResult JSFServer::CallBackAcceptEvent(JSFSocketAcceptMsg msg, Sint32 socketID, JSFSocketWrapper* pSocketWrapper)
{
	JSFResult ret = JSFRESULT_OK;

	printf("Accept Callback Message = %d", msg);

	switch (msg) {
		case JSFSOCKET_MSG_ACCEPT:
			if (pSocketWrapper != NULL)
			{
				return OnMsgAccept(socketID, pSocketWrapper);
			}
		/*
		case JSFSOCKET_MSG_ACCEPT_ERR: return onMsgAcceptErr(socketID, pSocketWrapper);
		case JSFSOCKET_MSG_ACCEPT_ERR_MEMORY_EMPTY: return onMsgAcceptErrMemoryEmpty(socketID, pSocketWrapper);
		case JSFSOCKET_MSG_ACCEPT_ERR_CONNECT_MAX: return onMsgAcceptErrConnectMax(socketID, pSocketWrapper);
		*/
	}
	return ret;
}

JSFResult JSFServer::OnMsgAccept(Sint32 socketID, JSFSocketWrapper* pSocketWrapper)
{
	JSFResult ret = JSFRESULT_OK;
	printf("OnMsgAccept socket = %d", socketID);
	JSFClientProperty *cprop = InitClientProperty(socketID, pSocketWrapper);
	pSocketWrapper->SetUserData(cprop);
	cprop->SetOnConnectTime();
	return ret;
}

JSFClientProperty* JSFServer::InitClientProperty(Sint32 socketID, JSFSocketWrapper* pSocketWrapper)
{
	JSFClientProperty *cprop = new JSFClientProperty();
	if (!cprop)
	{
		return false;
	}
	cprop->Init(this, socketID, pSocketWrapper);
	printf("cprop -> pSocketWrapper -> getRemoteIP() %s", cprop->GetSocketWrapper()->GetRemoteIP());
	JSFAutoMutexLocker lock(mMutexClientsList);
	mClientProps.insert(cprop);
	return cprop;
}

bool JSFServer::Run(Sint32 acceptNum, Sint32 connectNum, Sint32 threadNum, Sint32 idleTimeOut)
{
	if (acceptNum > 0) mAcceptNum = acceptNum;
	if (connectNum > 0) mConnectNum = connectNum;
	if (threadNum > 0) mOnPacketThreadNum = connectNum;
	if (idleTimeOut > 0) mIdleCutTimeout = idleTimeOut;
    pSocketsMgr->SetupListenerAndAcceptor(8802, 1, 1);
	for (int n = 0; n<mOnPacketThreadNum; ++n)
	{
		JSFOnPacketThread* pthread = new JSFOnPacketThread(0, this);
		if (!pthread->Run()) {
			return false;
		}
		mOnPacketThreadsList.push_back(pthread);
	}

	Sint32 readBufSize = 1024;
	Sint32 writeBufSize = 1024;
	if (!pSocketsMgr->Init(SOCK_MODE_TCP, this, connectNum, readBufSize, writeBufSize)) return 0;

	while (Polling())
	{
		JSF_SLEEP(1000);
	}
	return 0;

	
	return true;
}

bool JSFServer::Flush()
{
	JSFResult ret = pSocketsMgr->FlushAllEvent();
	if (JSFRESULT_FAILED(ret))
	{
		printf("FlushAllEvent error = %d", ret);
		return false;
	}
	return true;
}

bool JSFServer::Polling()
{
	Flush();

	JSFResult ret = pSocketsMgr->ReadAllEvent(mTimeout);
	if (JSFRESULT_FAILED(ret)) {
		printf("ReadAllEvent error = %d", ret);
		return false;
	}

	if (mIdleCutTimeout > 0)
	{
		//will add session expire handler.
	}

	return true;
}



