#ifndef JSF_CLIENT_PROPERTY_H_
#define JSF_CLIENT_PROPERTY_H_

#include <time.h>
#include "jsf_server_sockets_mgr.h"
#include "jsf_socket_wrapper.h"
#include "include\jsf_type_defs.h"

class JSFClientProperty
{
protected:
	JSFServer* mpOwner;
	Sint32 mSocketID;
	Sint32 mClientID;
	time_t mOnConnectTime;
	time_t mOnReadTime;
	JSFSocketWrapper* mpSocketWrapper;
public:
	virtual JSFServer* GetOwner();
	virtual Sint32 GetSocketID() const;
	virtual Sint32 GetClientID() const;

	virtual bool IsValid() const;

	time_t GetOnConnectTime() const;
	virtual time_t GetOnReadTime() const;
	virtual void SetOnConnectTime();
	virtual void SetOnReadTime();

	/*virtual bool IsConnectTimeoutDisable() const;

	virtual Sint8* GetRemoteIP();
	virtual Sint32 GetRemotePort();*/
	virtual void Init(JSFServer* owner, Sint32 socketID, JSFSocketWrapper* pSocketWrapper)
	{
		mpOwner = owner;
		//m_pAcceptProp = accept;
		mSocketID = socketID;
		mpSocketWrapper = pSocketWrapper;
		mOnConnectTime = GetTime();
	}

	time_t GetTime()
	{
		time_t nowTime;
		time(&nowTime);
		return nowTime;
	}

	virtual void Release() { mpOwner = nullptr; }
	JSFSocketWrapper* GetSocketWrapper() { return mpSocketWrapper; }

	JSFClientProperty() : mpOwner(nullptr), mSocketID(-1), mClientID(-1), mOnConnectTime(0), mOnReadTime(0) {}
	virtual ~JSFClientProperty() {}
};

#endif JSF_CLIENT_PROPERTY_H_