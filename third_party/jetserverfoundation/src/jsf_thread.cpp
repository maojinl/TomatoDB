#include "jsf_thread.h"

JSFThread_CallBackResult CALLBACK JSFThread::JSFThreadCallBack(void* iData)
{
	Uint32 RetCode;
	JSFThread *This;
	This = (JSFThread*)iData;
	if (This->mDelegateFunc == NULL)
	{
		RetCode = This->Run();
	}
	else
	{
		RetCode = This->mDelegateFunc(This);
	}
	This->CloseThreadHandle();
	return (JSFThread_CallBackResult)RetCode;
}

JSFThread::JSFThread(Sint32 createMode)
:mCreateMode(createMode), mTerminate(false), mDelegateFunc(NULL), mDelegatedData(NULL)
{
}


JSFThread::JSFThread(Sint32 createMode, JSFThreadDelegateFunc delegateFunc, void* pData)
	:mCreateMode(createMode), mTerminate(false), mDelegateFunc(delegateFunc), mDelegatedData(pData)
{
}

JSFThread::~JSFThread()
{
}

Uint32 JSFThread::Run()
{
	return 0;
}

void JSFThread::CloseThreadHandle()
{
#ifdef WIN32
	CloseHandle(mhThread);
#endif
	return;
}

void JSFThread::ThreadBegin()
{
#ifdef __LINUX__
	pthread_attr_t thread_attr;
	if (This->mCreateMode == 0)
	{
		pthread_create(&mhThread, NULL, JSFThreadCallBack, this);
	}
	else if (This->mCreateMode == 1)
	{
		pthread_attr_init(&thread_attr);
		pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&mhThread, &mCreateMode, JSFThreadCallBack, this);
	}
#elif WIN32
	mhThread = ::CreateThread(NULL, 0, JSFThreadCallBack, this, 0, &mThreadID);
#endif
}

JSFResult JSFThread::Release()
{
	bool ret = false;
	RequestQuit();
#ifdef __LINUX__
	ret = (pthread_join(mhThread, NULL) == 0);
#elif WIN32
	ret = (WaitForSingleObject(mhThread, INFINITE) != WAIT_FAILED);
#endif
	if (ret)
	{
		return JSFRESULT_NG;
	}
	return JSFRESULT_OK;
}

void JSFThread::RequestQuit()
{
	mbQuitReq = true;
}

bool JSFThread::IsContinue()
{
	return !mbQuitReq;
}

void* JSFThread::GetDelegatedData()
{
	return mDelegatedData;
}