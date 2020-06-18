#include "jsf_server_sockets_mgr.h"
#include "jsf_server.h"

using namespace std;

JSFThread_CallBackResult JSFServerSocketsMgr::JSFServerSocketsMgr_Acceptor(void* pThread)
{
	JSFServerSocketsMgr* pSockMgr = (JSFServerSocketsMgr*)((JSFThread*)pThread)->GetDelegatedData();
	return pSockMgr->Acceptor();
}

JSFServerSocketsMgr::JSFServerSocketsMgr()
{
}

JSFServerSocketsMgr::~JSFServerSocketsMgr()
{
}

JSFResult JSFServerSocketsMgr::Init(Sint32 how, JSFServer* pOwner, Sint32 connectNum, Sint32 writeBufSize, Sint32 readBufSize)
{
	JSFResult ret = JSFRESULT_OK;
	mSocketWrapperMemPool.SetupPool(connectNum);
	for (size_type i = 0; i < connectNum; i++)
	{
		JSFSocketWrapper* pSockWrp = mSocketWrapperMemPool.GetItem(i);
		pSockWrp->InitializeBuffer(writeBufSize, readBufSize);
	}

	printf("server init connect max = %d\n", connectNum);

	mConnMax = connectNum;
	mpOwner = pOwner;

#ifdef __LINUX__
	if ((mEPollFd = epoll_create(mConnMax)) < 0)
	{
		return ret;
	}
	mUseEPoll = true;
#endif
	mSockGUID = 0;
	mRunning = false;
	return ret;
}

Uint32 JSFServerSocketsMgr::Acceptor()
{
	JSFResult ret = JSFRESULT_OK;

	Sint8* pQueueBuf = NULL;

	Sint32 sockoptflag = 1;
	Sint32 result;
	//JSFSOCKEPollEvent epoll_event;

	mConnNowNum = 0;
	mIsAcceptRunning = true;

	while (mIsAcceptRunning)
	{
		JSFSocketWrapper* newSockWrapper = mSocketWrapperMemPool.Malloc();
		if (newSockWrapper == NULL) {
			ret = JSFRESULT_SOCKMGR_ERR_MEMORY_FULL;
			mpOwner->CallBackAcceptEvent(JSFSOCKET_MSG_ACCEPT_ERR_MEMORY_EMPTY, 0, nullptr);
			goto abort_1;
		}

		printf("acceptor thread Listen Port = %d\n", mListenPort);

		ret = mpListenSocket->Accept(mListenPort, newSockWrapper);
		if (JSFRESULT_FAILED(ret)) {
			mpOwner->CallBackAcceptEvent(JSFSOCKET_MSG_ACCEPT_ERR, -1, nullptr);
			mSocketWrapperMemPool.Free(newSockWrapper);
			goto abort_1;
		}

		newSockWrapper->SetRemoteIPAndPort();
		newSockWrapper->SetNonBlock();
		
		printf("accepted connection from %s port=%d\n", newSockWrapper->GetRemoteIP(), newSockWrapper->GetRemotePort());

		if (mSockOpt == JSF_TCP_NODELAY) {
			sockoptflag = 1;
			result = newSockWrapper->SetSocketOption(IPPROTO_TCP, JSF_TCP_NODELAY, (char*)&sockoptflag, sizeof(sockoptflag));
			if (result < 0) {
				printf("Couldn't setsockopt(TCP_NODELAY)\n");
			}
		}

#ifdef __LINUX__

		//Planning to use epoll_ctl for epoll functions
		
#endif//__LINUX__

		mSockListMutex.Lock();
		mConnNowNum++;

		if (mConnNowNum > mSockListCount) {
			mReserve0 = (Sint64)newSockWrapper;
			ret = mpOwner->CallBackAcceptEvent(JSFSOCKET_MSG_ACCEPT_ERR_CONNECT_MAX, -1, newSockWrapper);
			mConnNowNum--;

#ifdef __LINUX__
			if (epoll_ctl(mEPollFd, EPOLL_CTL_DEL, newSockWrapper->GetSocket().GetRawSocket(), NULL) < 0) {
				printf("epoll_ctl del error!\n");
			}
#endif
			newSockWrapper->Close();
			mSocketWrapperMemPool.Free(newSockWrapper);
			if (!JSFRESULT_FAILED(ret)) {
				continue;
			}
			goto abort_1;
		}

		newSockWrapper->SetConnectID(mSockGUID);
		mSockGUID++;
		mSockListCount++;
		newSockWrapper->SetOwnerID(mpListenSocket->GetConnectID());
		mSockList.push_back(newSockWrapper);
		mSockListMutex.Unlock();

		mpOwner->CallBackAcceptEvent(JSFSOCKET_MSG_ACCEPT, newSockWrapper->GetConnectID(), newSockWrapper);
		newSockWrapper->GetSocket().SetStatusBit(true, JSFSOCK_STATUS_SOCK_ENABLE);
	}
abort_1:
	mIsAcceptRunning = false;
	return ret;
}

JSFResult JSFServerSocketsMgr::SetupListenerAndAcceptor(const Uint16 cListenPort, Sint32 SockOpt, Sint32 listenerID)
{
	JSFResult ret = JSFRESULT_OK;

	printf("server accept connect max = %d, Port=%d\n", this->mConnMax, cListenPort);

	mpListenSocket = new JSFSocketWrapper();
	mpListenSocket->SetConnectID(listenerID);
	ret = mpListenSocket->InitSocket(SOCK_MODE_TCP);
	if (JSFRESULT_FAILED(ret)) 
	{
		return ret;
	}
	mListenPort = cListenPort;
	mSockOpt = SockOpt;

	this->mpAcceptorThread = new JSFThread(0, JSFServerSocketsMgr_Acceptor, (void*)this);

	mpAcceptorThread->ThreadBegin();
	return ret;
}

#ifdef WIN32
JSFResult JSFServerSocketsMgr::SocketsPoll_Platform_Targeted(Sint32& retSelect, Sint32 TimeOut)
{
	JSFSockFdSet Rmask;
	JSFSockFdSet Wmask;
	JSFSockFdSet Emask;
	JSFSockTimeVal Tv;
	Sint32 ClientNum;
	JSFResult ret = JSFRESULT_OK;

	Tv.tv_sec = TimeOut / 1000;
	Tv.tv_usec = (TimeOut % 1000);

	JSFSOCK_FD_ZERO(&Rmask);
	JSFSOCK_FD_ZERO(&Wmask);
	JSFSOCK_FD_ZERO(&Emask);

	ClientNum = mSockList.size();
	if (ClientNum == 0) {
		JSF_SLEEP(TimeOut);
		retSelect = 0;
		return ret;
	}
	else
	{
		for (SocketWrappersList::iterator ite = mSockList.begin(); ite != mSockList.end(); ite++ )
		{
			JSFSocketWrapper* pSockW = *ite;
			JSFSOCK_FD_SET(pSockW->GetSocket().GetRawSocket(), &Rmask);
			JSFSOCK_FD_SET(pSockW->GetSocket().GetRawSocket(), &Wmask);
			JSFSOCK_FD_SET(pSockW->GetSocket().GetRawSocket(), &Emask);
		}
	}

	retSelect = select(FD_SETSIZE, &Rmask, NULL, NULL, &Tv);
	if (retSelect < 0)
	{
		ret = JSFRESULT_SOCKMGR_ERR_POLL;
		return ret;
	}

	for (SocketWrappersList::iterator ite = mSockList.begin(); ite != mSockList.end(); ite++)
	{
		JSFSocketWrapper* pSockW = *ite;
		pSockW->mRevents = 0;
		RawSocket socket = pSockW->GetSocket().GetRawSocket();
		if (JSFSOCK_FD_ISSET(socket, &Rmask)) {
			pSockW->mRevents |= JSFSOCK_POLLIN;
		}
		if (JSFSOCK_FD_ISSET(socket, &Emask)) {
			pSockW->mRevents |= JSFSOCK_POLLERR;
		}
	}
	return ret;
}
#elif __LINUX__

JSFResult JSFServerSocketsMgr::SocketsPoll_Platform_Targeted(Sint32& retSelect, Sint32 TimeOut)
{
	JSFResult ret = JSFRESULT_OK;
	return ret;
}

#endif

JSFResult JSFServerSocketsMgr::ReadAllEvent(Sint32 TimeOut)
{
	JSFResult ret = JSFRESULT_OK;
	JSFSocketWrapper* pSockW;
	Sint32 ClientNum;
	Sint32 SeekSize = 0;

	ClientNum = mSockList.size();
	if (ClientNum == 0) {
		JSF_SLEEP(TimeOut);
		return ret;
	}
	int retSelect;
	ret = SocketsPoll_Platform_Targeted(retSelect, TimeOut);
	if (JSFRESULT_FAILED(ret)) 
	{
		printf("poll error\n");
		return ret;
	}

	for (SocketWrappersList::iterator ite = mSockList.begin(); ite != mSockList.end(); ite++)
	{
		pSockW = *ite;
		if (pSockW == nullptr)
		{
			printf("[ReadAllEvent] wrong socket\n");
			continue;
		}

		if (pSockW->GetSocket().CheckStatusBit(JSFSOCK_STATUS_SEND_CLOSE_REGIST))
		{
			printf("[ReadAllEvent] send regist socket close\n");
			mpOwner->CallBackSocketEvent(pSockW->GetUserData(), JSFSOCKET_MSG_SOCKET_CLOSE, nullptr);
			pSockW->GetSocket().SetStatusBit(true, JSFSOCK_STATUS_CLOSE_REGIST);
		}

		if (pSockW->GetSocket().CheckStatusBit(JSFSOCK_STATUS_SOCK_ENABLE))
		{
			if (!pSockW->GetSocket().CheckStatusBit(JSFSOCK_STATUS_CLOSE_REGIST))
			{
				if (pSockW->mRevents & JSFSOCK_POLLIN)
				{
					ret = pSockW->Read();
					if (JSFRESULT_FAILED(ret)) 
					{
						pSockW->GetSocket().SetStatusBit(true, JSFSOCK_STATUS_CLOSE_REGIST);
					}
				}
				else if (pSockW->mRevents & JSFSOCK_POLLERR)
				{
					pSockW->GetSocket().SetStatusBit(true, JSFSOCK_STATUS_CLOSE_REGIST);
				}
			}
		}

		if (pSockW->GetSocket().CheckStatusBit(JSFSOCK_STATUS_CLOSE_REGIST))
		{
			mSockListMutex.Lock();
			printf("[ReadAllEvent] Connect Close(ConnectID OwnerID) %d %d\n", pSockW->GetConnectID(), pSockW->GetOwnerID());
#ifdef __LINUX__
			if (epoll_ctl(mEPollFd, EPOLL_CTL_DEL, pSockW->GetSocket().GetRawSocket(), NULL) < 0)
			{
				printf("epoll_ctl del error!\n");
			}
			else {
				printf("epoll_ctl del ok!\n");
			}
			//epoll stuffs
#endif
			pSockW->Close();
			mSockList.remove(pSockW);
			mSockListMutex.Unlock();
		}
	}

	if (mRunning == FALSE)
	{
		ret = JSFRESULT_NG;
	}
	return ret;
}

JSFResult JSFServerSocketsMgr::FlushAllEvent()
{
	JSFResult ret = JSFRESULT_OK;
	JSFSocketWrapper* pSockW;

	mSockListMutex.Lock();

	for (SocketWrappersList::iterator ite = mSockList.begin(); ite != mSockList.end(); ite++)
	{
		pSockW = *ite;
		if (pSockW == nullptr)
		{
			printf("[ReadAllEvent] wrong socket\n");
			continue;
		}

		if (!pSockW->GetSocket().CheckStatusBit(JSFSOCK_STATUS_CLOSE_REGIST))
		{
			ret = pSockW->Flush();
		}
	}

	mSockListMutex.Lock();
	return ret;
}