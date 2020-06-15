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
#include "util\arena.h"
#include "util\arena_ex.h"

using namespace std;

typedef long long int Sint64;
namespace leveldb {
struct MemoryBHeader {
  MemoryBHeader* mpNext;
  MemoryBHeader* mpPrev;
  Sint64 mSize;
};

class MemoryHandler {
 private:
  const int MEMORY_ALIGN;
  Sint64 mReal;
  MemoryBHeader* mpAlloc;
  MemoryBHeader* mpHead;
  void* mpTail;
  Sint64 mBlocks;
  std::atomic<size_t> memory_usage_;  // Used Memory Size
  Sint64 mAllocBaseSize;              // AllocBase Size

  char* skip_header(char* p) {
    char* ret = (char*)(reinterpret_cast<Sint64>(p) + mAlignedHeaderSize);
    return ret;
  }

  MemoryBHeader* get_header(void* p) {
    MemoryBHeader* ret =
        (MemoryBHeader*)(reinterpret_cast<Sint64>(p) - mAlignedHeaderSize);
    return ret;
  }

 public:
  Sint64 mAlignedHeaderSize;
  MemoryHandler()
      : mReal(0),
        mpAlloc(0),
        mpHead(0),
        mpTail(0),
        mBlocks(0),
        memory_usage_(0),
        mAllocBaseSize(0),
        MEMORY_ALIGN((sizeof(void*) > 8) ? sizeof(void*) : 8) {
    mAlignedHeaderSize = this->MemoryAligned(sizeof(MemoryBHeader));
  }
  virtual ~MemoryHandler() {}

  size_t MemoryUsage() const {
    return memory_usage_.load(std::memory_order_relaxed);
  }

  Sint64 MemoryAligned(Sint64 x) {
    Sint64 ret = (x + (MEMORY_ALIGN - 1)) & (-MEMORY_ALIGN);
    return ret;
  }

  int InitHeap(void* iHeap, size_t size);
  char* Allocate(size_t size, bool aligned = false);
  char* AllocateAligned(size_t size);
  int Destroy(void*);
  size_t Available();
};

#define USE_ARENA_EX

class MemoryPool {
 public:
  MemoryPool(size_t bytes) { Initialize(bytes); }
  MemoryPool(const MemoryPool&) = delete;
  MemoryPool& operator=(const MemoryPool&) = delete;
  ~MemoryPool() {}

#ifdef USE_ARENA
 private:
  Arena arena_;

 public:
  void Initialize(size_t bytes) {}
  char* Allocate(size_t bytes) { return arena_.Allocate(bytes); }
  char* AllocateAligned(size_t bytes) { return arena_.AllocateAligned(bytes); }
  size_t MemoryUsage() const { return arena_.MemoryUsage(); }
#elif defined USE_MEMORY_HANDLER
 private:
  MemoryHandler memHandler;
  vector<char> memBuffer;
  MemoryHandler memHandlerEx;
  vector<char> memBufferEx;
  void Initialize(size_t bytes) {
    bytes += bytes / 1024;
    memBuffer.resize(bytes);
    memHandler.InitHeap(&memBuffer[0], bytes);
  }

 public:
  char* Allocate(size_t bytes) { 
    if (memHandler.Available() >= bytes) {
      return memHandler.Allocate(bytes);
    } else {
      if (!memHandlerEx.initialized) {
        size_t bufferBytes = memHandler.MemoryAligned(bytes + memHandlerEx.mAlignedHeaderSize);
        memBufferEx.resize(bufferBytes);
        memHandlerEx.InitHeap(&memBufferEx[0], bufferBytes);
      }
      return memHandlerEx.Allocate(bytes);
    }  
  }
  char* AllocateAligned(size_t bytes) {
    bytes = memHandler.MemoryAligned(bytes);
    return Allocate(bytes);
  }
  size_t MemoryUsage() const { return memHandler.MemoryUsage() + memHandlerEx.MemoryUsage(); }
#elif defined USE_ARENA_EX
 private:
  ArenaEx arenaEx_;

 public:
  void Initialize(size_t bytes) { arenaEx_.Initialize(bytes); }
  char* Allocate(size_t bytes) { return arenaEx_.Allocate(bytes); }
  char* AllocateAligned(size_t bytes) {
    return arenaEx_.AllocateAligned(bytes);
  }
  size_t MemoryUsage() const { return arenaEx_.MemoryUsage(); }
#endif

};

}  // namespace leveldb

#endif /* MEMORY_HANDLER_H_ */