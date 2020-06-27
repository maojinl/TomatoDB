#include "jsf_onpacket_thread.h"

JSFOnPacketThread::JSFOnPacketThread(Sint32 createMode, JSFServer *pServer, size_t queLimit /*= 1024*/) 
	:JSFThread(createMode), mpServer(pServer), mpClientProp(NULL), mQueueLimit(queLimit)
{
}

JSFOnPacketThread::~JSFOnPacketThread()
{
	for (JSFPacketQueue::iterator ite = mPacketQueue.begin(); ite != mPacketQueue.end(); ite++)
	{
		delete (*ite);
	}
	mPacketQueue.clear();
}

bool JSFOnPacketThread::IsEmpty() const
{
	return mPacketQueue.empty();
}

void JSFOnPacketThread::PostPacket(JSFPacket* pPacket)
{
	mPacketQueueMutex.Lock();
	mPacketQueue.push_back(pPacket);
	mPacketQueueMutex.Unlock();
}

Uint32 JSFOnPacketThread::Run()
{
	while (IsContinue())
	{
		if (mPacketQueue.size() < 1)
		{
			JSF_SLEEP(1000);
			continue;
		}
		JSFPacket* pPacket = mPacketQueue.front();
		size_t bufSize = pPacket->GetPacketData().size();
		char* s = new char[bufSize + 1];
		strcpy_s(s, bufSize, &(pPacket->GetPacketData()[0]));
		s[bufSize] = 0;
		printf("[JSFOnPacketThread::Run] packet handling, packet is %s", s);
		mPacketQueueMutex.Lock();
		mPacketQueue.pop_front();
		mPacketQueueMutex.Unlock();
        delete pPacket;
	}
	return 0;
}