#ifndef JSF_ONPACKET_THREAD_H_
#define JSF_ONPACKET_THREAD_H_

#include <deque>
#include "jsf_thread.h"
#include "jsf_packet.h"
#include "jsf_type_defs.h"
#include "jsf_thread_lock.h"

using namespace std;

class JSFServer;
class JSFClientProperty;

class JSFOnPacketThread : public JSFThread
{
	typedef JSFThread super;
	typedef deque<JSFPacket*> JSFPacketQueue;
public:
	JSFServer* mpServer;
	JSFPacketQueue mPacketQueue;
	void *m_pUser;
	JSFClientProperty* mpClientProp;
	size_t mQueueLimit;
	JSFThreadLock mPacketQueueMutex;
public:
	virtual bool IsEmpty() const;
	virtual void PostPacket(JSFPacket* pPacket);
public:
	virtual Uint32 Run();
public:
	JSFOnPacketThread(Sint32 createMode, JSFServer* pServer, size_t queLimit = 1024);
	virtual ~JSFOnPacketThread();
};

#endif