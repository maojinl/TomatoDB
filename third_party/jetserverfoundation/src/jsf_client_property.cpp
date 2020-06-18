#include "jsf_client_property.h"

JSFServer* JSFClientProperty::GetOwner()
{
	return mpOwner; 
}

Sint32 JSFClientProperty::GetSocketID() const
{
	return mSocketID;
}

Sint32 JSFClientProperty::GetClientID() const
{
	return mClientID;
}

bool JSFClientProperty::IsValid() const
{
	return mpOwner != NULL;
}

time_t JSFClientProperty::GetOnConnectTime() const 
{
	return mOnConnectTime;
}

time_t JSFClientProperty::GetOnReadTime() const
{
	return mOnReadTime; 
}
void JSFClientProperty::SetOnConnectTime() 
{ 
	mOnConnectTime = GetTime();
}

void JSFClientProperty::SetOnReadTime()
{
	mOnReadTime = GetTime();
}