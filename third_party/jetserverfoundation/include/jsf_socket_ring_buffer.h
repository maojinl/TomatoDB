#ifndef JSF_SOCKET_RING_BUFFER_H_
#define JSF_SOCKET_RING_BUFFER_H_

#include "jsf_type_defs.h"

class JSFSocketRingBuffer
{
public:
	JSFSocketRingBuffer()
	{
	}

	char* mpBuf;
	Sint32 mBufSize;
	Sint32 mBufWritePoint;
	Sint32 mBufReadPoint;
	bool Initialize(size_t bufSize)
	{
		try
		{
			mBufWritePoint = 0;
			mBufReadPoint = 0;
			mBufSize = bufSize;
			mpBuf = new Sint8[mBufSize];
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	//always leave one byte unused for not let read point write point meets again 
	//rather than empty buffer
	size_t GetBufferFreeSpaces(size_t& headSpace, size_t tailSpace)
	{
		if (mBufWritePoint > mBufReadPoint)
		{
			if (mBufReadPoint == 0)
			{
				headSpace = 0;
				tailSpace = mBufSize - mBufWritePoint - 1;
				return headSpace + tailSpace;
			}
			else
			{
				headSpace = mBufReadPoint - 1;
				tailSpace = mBufSize - mBufWritePoint;
				return headSpace + tailSpace;
			}
		}
		else if (mBufWritePoint < mBufReadPoint)
		{
			headSpace = 0;
			tailSpace = mBufReadPoint - mBufWritePoint - 1;
			return headSpace + tailSpace;
		}
		else
		{
			if (mBufReadPoint > 0)
			{
				ResetBufferPointers();
			}
			headSpace = 0;
			tailSpace = mBufSize - 1;
			return tailSpace;
		}
	}

	size_t GetBufferFilledSpaces(size_t& headSpace, size_t tailSpace)
	{
		if (mBufReadPoint < mBufWritePoint )
		{
			headSpace = 0;
			tailSpace = mBufReadPoint - mBufWritePoint;
			return headSpace + tailSpace;
		}
		else if (mBufReadPoint > mBufWritePoint)
		{
			headSpace = mBufSize - mBufReadPoint;
			tailSpace = mBufReadPoint - mBufWritePoint - 1;
			return headSpace + tailSpace;
		}
		else
		{
			if (mBufReadPoint > 0)
			{
				ResetBufferPointers();
			}
			headSpace = 0;
			tailSpace = 0;
			return 0;
		}
	}

	char* GetWritePosition()
	{
		return &mpBuf[mBufWritePoint];
	}

	char* GetReadPosition()
	{
		return &mpBuf[mBufReadPoint];
	}

	char* GetStartPosition()
	{
		return mpBuf;
	}

	void AddWritePoint(size_t addLength)
	{
		mBufWritePoint = (mBufWritePoint + addLength) % mBufSize;
	}

	void AddReadPoint(size_t addLength)
	{
		mBufReadPoint = (mBufReadPoint + addLength) % mBufSize;
		if (mBufReadPoint == mBufWritePoint)
		{
			ResetBufferPointers();
		}
	}

	void ResetBufferPointers()
	{
		mBufWritePoint = 0;
		mBufReadPoint = 0;
	}

	virtual ~JSFSocketRingBuffer()
	{
		mBufWritePoint = 0;
		mBufReadPoint = 0;
		mBufSize = 0;
		if (mpBuf != NULL)
		{
			delete[] mpBuf;
			mpBuf = NULL;
		}
	}
};

#endif