// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/arena.h"
#include "util/memoryhandler.h"

#include "gtest/gtest.h"
#include "util/random.h"

namespace leveldb {

TEST(ArenaTest, Empty) { Arena arena; }

TEST(ArenaTest, Simple) {
  std::vector<std::pair<size_t, char*>> allocated;
  Arena arena;
  const int N = 100000;
  size_t bytes = 0;
  Random rnd(301);
  for (int i = 0; i < N; i++) {
    size_t s;
    if (i % (N / 10) == 0) {
      s = i;
    } else {
      s = rnd.OneIn(4000)
              ? rnd.Uniform(6000)
              : (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
    }
    if (s == 0) {
      // Our arena disallows size 0 allocations.
      s = 1;
    }
    char* r;
    if (rnd.OneIn(10)) {
      r = arena.AllocateAligned(s);
    } else {
      r = arena.Allocate(s);
    }

    for (size_t b = 0; b < s; b++) {
      // Fill the "i"th allocation with a known bit pattern
      r[b] = i % 256;
    }
    bytes += s;
    allocated.push_back(std::make_pair(s, r));
    ASSERT_GE(arena.MemoryUsage(), bytes);
    if (i > N / 10) {
      ASSERT_LE(arena.MemoryUsage(), bytes * 1.10);
    }
  }
  for (size_t i = 0; i < allocated.size(); i++) {
    size_t num_bytes = allocated[i].first;
    const char* p = allocated[i].second;
    for (size_t b = 0; b < num_bytes; b++) {
      // Check the "i"th allocation for the known bit pattern
      ASSERT_EQ(int(p[b]) & 0xff, i % 256);
    }
  }
}

TEST(MemoryHandlerTest, Simple) {
  MemoryHandler mem;
  vector<char> v;
  const int N = 1024 * 1024;
  v.resize(N);
  char* p = &v[0];
  mem.InitHeap(p, N);
  void* p1 = mem.AllocateAligned(100);
  size_t expectSize = 104 + mem.MemoryAligned(sizeof(MemoryBHeader));
  ASSERT_EQ(mem.MemoryUsage(), expectSize);
  mem.Destroy(p1);
  ASSERT_EQ(mem.MemoryUsage(), 0);

   void* pp[1024];
   for (int i = 0; i < 1024; i++) {
    pp[i] = mem.AllocateAligned(1000);
  }
   ASSERT_EQ(mem.MemoryUsage(), N);
   ASSERT_EQ(mem.Available(), 0);
   mem.Destroy(pp[999]);
   mem.Destroy(pp[500]);
   ASSERT_EQ(mem.MemoryUsage(), N - 2048);
   ASSERT_EQ(mem.Available(), 1000);
   mem.Destroy(pp[998]);
   ASSERT_EQ(mem.Available(), 2024);
}

}  // namespace leveldb

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
