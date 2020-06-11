/*
 * memoryhandler.h
 *
 *  Created on: Jul 19, 2013
 *      Author: mao
 */

#ifndef MEMORY_HANDLER_H_
#define MEMORY_HANDLER_H_

#include <stddef.h>
#include <xatomic.h>
#include <atomic>

using namespace std;

typedef long long int Sint64;

struct MemoryBHeader
{
	MemoryBHeader*  mpNext;
	MemoryBHeader*  mpPrev;
	Sint64	mSize;
};

class MemoryHandler {
 private:
	const int MEMORY_ALIGN;
	Sint64			mReal;
    MemoryBHeader*  mpAlloc;
    MemoryBHeader*  mpHead;
	void*           mpTail;
	Sint64          mBlocks;
    std::atomic<size_t> memory_usage_;  // Used Memory Size
    Sint64          mAllocBaseSize;  // AllocBase Size
    Sint64			mAlignedHeaderSize;

    void* skip_header(void* p)
    {
    	void* ret = (void*)(reinterpret_cast<Sint64>(p) + mAlignedHeaderSize);
    	return ret;
    }

    MemoryBHeader* get_header(void* p)
    {
    	MemoryBHeader* ret = (MemoryBHeader*)(reinterpret_cast<Sint64>(p) - mAlignedHeaderSize);
    	return ret;
    }

public:
    MemoryHandler()
        : mReal(0),
          mpAlloc(0),
          mpHead(0),
          mpTail(0),
       mBlocks(0),
       memory_usage_(0),
       mAllocBaseSize(0),
       MEMORY_ALIGN((sizeof(void*) > 8) ? sizeof(void*) : 8)
	{
        mAlignedHeaderSize = this->MemoryAligned(sizeof(MemoryBHeader));
	}
    virtual ~MemoryHandler()
	{
	}

	size_t MemoryUsage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

   Sint64 MemoryAligned(Sint64 x) {
      Sint64 ret = (x + (MEMORY_ALIGN - 1)) & (-MEMORY_ALIGN);
      return ret;
    }

	int InitHeap(void* iHeap, size_t size);
    void* Allocate(size_t size, bool aligned = false);
    void* AllocateAligned(size_t size);
	int Destroy(void*);
    size_t Available();
};

#endif /* MEMORY_HANDLER_H_ */
