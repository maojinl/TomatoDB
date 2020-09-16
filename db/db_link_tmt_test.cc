// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <atomic>
#include <string>
#include "util/trietree.h"
#include "leveldb/db_link_tmt.h"
#include "util/testutil.h"
#include "leveldb/slice.h"

#include "gtest/gtest.h"

using namespace std;
using namespace leveldb;

namespace tomatodb {

static const int kNumThreads = 4;
static const int kTestSeconds = 10;
static const int kNumKeys = 1000;

TEST(TrieTreeTest, Simple) {
  TrieTree t;
  string s("a");
  TrieTree* pt = t.AddWord(Slice(s));
  ASSERT_EQ(t.FindWord(Slice(s)), pt);
  ASSERT_EQ(pt->GetLevel(), 1);
  ASSERT_EQ(pt->GetChar(), 'a');
  ASSERT_EQ(pt->GetEdgesCount(), 0);
  string s1("abc");
  TrieTree* pt1 = t.AddWord(Slice(s1));
  ASSERT_EQ(t.FindWord(Slice(s1)), pt1);
  ASSERT_EQ(pt->GetEdgesCount(), 1);
  ASSERT_EQ(pt1->GetLevel(), 3);
  ASSERT_EQ(pt1->GetChar(), 'c');
}

TEST(TrieTreeTest, RemoveNodeTest) {
  TrieTree t;
  string s("a");
  TrieTree* pt = t.AddWord(Slice(s));
  TrieTree::RemoveNode(&t, pt);
  ASSERT_EQ(pt, nullptr);

  string s0("a");
  pt = t.AddWord(Slice(s0));
  string s1("abc");
  TrieTree* pt1 = t.AddWord(Slice(s1));
  string s2("bcd");
  TrieTree* pt2 = t.AddWord(Slice(s2));
  ASSERT_EQ(TrieTree::RemoveNode(&t, pt), false);
  ASSERT_NE(pt, nullptr);
  ASSERT_EQ(t.FindWord(Slice(s0)), pt);
  ASSERT_EQ(pt->GetEdgesCount(), 1);
  TrieTree::RemoveNode(&t, pt1);
  ASSERT_EQ(t.FindWord(Slice(s0)), nullptr);
  ASSERT_EQ(pt1, nullptr);
  ASSERT_EQ(t.GetEdgesCount(), 1);
}

TEST(DBLinkTmtTest, Simple) { 
  TmtDBLink link("testdb", "friends"); 
  
}

}  // namespace tomatodb

int main(int argc, char** argv) {

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
