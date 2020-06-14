// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_ARENA_EX_H_
#define STORAGE_LEVELDB_UTIL_ARENA_EX_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

class ArenaEx {
 public:
  ArenaEx()
      : alloc_ptr_(nullptr),
        alloc_bytes_remaining_(0),
        memory_usage_(0),
        MEMORY_ALIGN((sizeof(void*) > 8) ? sizeof(void*) : 8) {}

  ArenaEx(const ArenaEx&) = delete;
  ArenaEx& operator=(const ArenaEx&) = delete;

  ~ArenaEx() {
    for (size_t i = 0; i < blocks_.size(); i++) {
      delete[] blocks_[i];
    }
  }

  // Return a pointer to a newly allocated memory block of "bytes" bytes.
  char* Allocate(size_t bytes) {
    // The semantics of what to return are a bit messy if we allow
    // 0-byte allocations, so we disallow them here (we don't need
    // them for our internal use).
    char* result;
    assert(bytes > 0);
    if (bytes <= alloc_bytes_remaining_) {
      result = alloc_ptr_;
      alloc_ptr_ += bytes;
      alloc_bytes_remaining_ -= bytes;
    } else {
      result = new char[bytes];
      blocks_.push_back(result);
      alloc_bytes_remaining_ = 0;
    }
    memory_usage_.fetch_add(bytes, std::memory_order_relaxed);
    return result;
  }

  // Allocate memory with the normal alignment guarantees provided by malloc.
  char* AllocateAligned(size_t bytes) {
    bytes = MemoryAligned(bytes);
    return Allocate(bytes);
  }

  // Returns an estimate of the total memory usage of data allocated
  // by the arena.
  size_t MemoryUsage() const {
    return memory_usage_.load(std::memory_order_relaxed);
  }

  size_t MemoryAligned(size_t x) {
    size_t ret = (x + (MEMORY_ALIGN - 1)) & (-MEMORY_ALIGN);
    return ret;
  }

  void Initialize(size_t size) {
    size = MemoryAligned(size);
    char* result = new char[size];
    blocks_.push_back(result);
    alloc_bytes_remaining_ = size;
    alloc_ptr_ = &blocks_[0][0];
  };

 private:
  // Allocation state
  char* alloc_ptr_;
  size_t alloc_bytes_remaining_;

  // Array of new[] allocated memory blocks
  std::vector<char*> blocks_;

  // Total memory usage of the arena.
  //
  // TODO(costan): This member is accessed via atomics, but the others are
  //               accessed without any locking. Is this OK?
  std::atomic<size_t> memory_usage_;

  const int MEMORY_ALIGN;
};
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_ARENA_EX_H_