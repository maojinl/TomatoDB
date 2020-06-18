#ifndef JSF_THREAD_LOCK_H_
#define JSF_THREAD_LOCK_H_

#ifdef WIN32
#include <windows.h>
class JSFThreadLock
{
	CRITICAL_SECTION m_Lock;
public:
	JSFThreadLock() { InitializeCriticalSection(&m_Lock); };
	~JSFThreadLock() { DeleteCriticalSection(&m_Lock); };
	void Lock() { EnterCriticalSection(&m_Lock); };
	void Unlock() { LeaveCriticalSection(&m_Lock); };
};
#elif __LINUX__
#include<pthread.h> 
class JSFThreadLock
{
	pthread_mutex_t m_Mutex;
public:
	JSFThreadLock() { pthread_mutex_init(&m_Mutex, NULL); };
	~JSFThreadLock() { pthread_mutex_destroy(&m_Mutex); };
	void Lock() { pthread_mutex_lock(&m_Mutex); };
	void Unlock() { pthread_mutex_unlock(&m_Mutex); };
};
#endif

class JSFAutoMutexLocker
{
protected:
	JSFThreadLock &mLock;
public:
	JSFAutoMutexLocker(JSFThreadLock& lock) : mLock(lock)
	{
		mLock.Lock();
	}
	~JSFAutoMutexLocker()
	{
		mLock.Unlock();
	}
};

#endif