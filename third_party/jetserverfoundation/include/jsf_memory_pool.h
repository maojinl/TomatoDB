#ifndef JSF_MEMORY_POOL_H_
#define JSF_MEMORY_POOL_H_

typedef int size_type;

template<typename _Ty>
class JSFMemoryPool
{

private:
	typedef _Ty T;
	size_type mMaxEntry;
	T* mTable;
	size_type* mAllocateTable;
	size_type mReadPoint;
	size_type mWritePoint;
	size_type mAssigned;
public:
	size_type GetMaxEntry()
	{
		return mMaxEntry;
	}

	JSFMemoryPool(size_type n = 0)
	{
		mReadPoint = 0;
		mWritePoint = 0;
		mAssigned = 0;
		mMaxEntry = n;
		mAllocateTable = 0;
		mTable = 0;
	}
	
	int SetupPool(size_type n)
	{
		if (mMaxEntry>0)
			return -1;

		mReadPoint = 0;
		mWritePoint = 0;
		mMaxEntry = n;
		mAllocateTable = 0;
		mTable = 0;
		if (mMaxEntry>0)
		{
			mTable = new T[mMaxEntry];
			mAllocateTable = new size_type[mMaxEntry];

			for (size_type i = 0; i<mMaxEntry; i++)
			{
				mAllocateTable[i] = i;
			}
		}
		return 0;
	}

	T* GetItem(size_type index)
	{
		return &mTable[index];
	}

	T* Malloc()
	{
		if (mTable==0)
			return NULL;
			
		if (mAssigned>= mMaxEntry)
			return NULL;
			
		if (mMaxEntry ==0)
			return NULL;
			
		size_type assignNum = mAllocateTable[mReadPoint];
		mAssigned++;
		mReadPoint = (mReadPoint + 1)%mMaxEntry;
		return &mTable[assignNum];
	}
	
	int Free(T* pDelete)
	{
		if (mTable==0)
			return -1;
			
		if (mAssigned<=0)
			return -1;
			
		if (mMaxEntry==0)
			return -1;
			
		size_type deleteNo = pDelete - mTable;
		
		if (deleteNo<0)
			return -1;
			
		mAssigned--;
		mAllocateTable[mWritePoint] = deleteNo;	
		mWritePoint = (mWritePoint + 1)% mMaxEntry;
		return 0;
	}
	
	void Reset()
	{
		mReadPoint = 0;
		mWritePoint = 0;
		mAssigned = 0;
	}

	virtual ~JSFMemoryPool()
	{
		if (mMaxEntry >=0)
		{
			delete [] mTable;
			delete [] mAllocateTable;
			mTable = 0;
			mAllocateTable = 0;
			mReadPoint = 0;
			mWritePoint = 0;
			mAssigned = 0;
			mWritePoint = 0;
			mMaxEntry = 0;
		}
	}
};

#endif