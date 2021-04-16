// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <atomic>
#include <string>
#include <vector>
#include "util/trietree.h"
#include "util/sorted_vector.h"
#include "leveldb/db_link_tmt.h"
#include "util/testutil.h"
#include "leveldb/slice.h"

#include "gtest/gtest.h"

using namespace std;
using namespace leveldb;

namespace tomatodb {

static const int kNumThreads = 4;
static const int kTestRounds = 1000;
static const int kNumKeys = 10000000;
static const int kNumLinks = 200;

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
  vector<string> l;
  string s1 = "abc";
  string s2 = "bcd";
  string s3 = "ghi";
  l.push_back(s2);
  l.push_back(s3);
  link.AddLinks(s1, l);
  l.clear();
  l.push_back(s1);
  l.push_back(s3);
  link.AddLinks(s2, l);
  l.clear();
  l.push_back(s1);
  l.push_back(s2);
  link.AddLinks(s3, l);

  vector<string*> ret;
  link.GetLinksReverse(s1, ret);
  ASSERT_EQ(ret.size(), 2);
  for (int i = 0; i < ret.size(); i++) {
    bool flag = ret[i]->compare(s2) == 0 || ret[i]->compare(s3) == 0;
    ASSERT_EQ(flag, true);
  }
  for (int i = 0; i < ret.size(); i++) {
    delete ret[i];
  }
  ret.clear();

  link.GetLinksReverse(s2, ret);
  ASSERT_EQ(ret.size(), 2);
  for (int i = 0; i < ret.size(); i++) {
    bool flag = ret[i]->compare(s1) == 0 || ret[i]->compare(s3) == 0;
    ASSERT_EQ(flag, true);
  }
  for (int i = 0; i < ret.size(); i++) {
    delete ret[i];
  }
  ret.clear();

  link.GetLinksReverse(s3, ret);
  ASSERT_EQ(ret.size(), 2);
  for (int i = 0; i < ret.size(); i++) {
    bool flag = ret[i]->compare(s1) == 0 || ret[i]->compare(s2) == 0;
    ASSERT_EQ(flag, true);
  }
  for (int i = 0; i < ret.size(); i++) {
    delete ret[i];
  }
  ret.clear();

  link.RemoveLinks(s2);
  link.RemoveLinks(s1);

  link.GetLinks(s2, ret);
  ASSERT_EQ(ret.size(), 0);
  link.GetLinks(s1, ret);
  ASSERT_EQ(ret.size(), 0);

  link.GetLinksReverse(s2, ret);
  ASSERT_EQ(ret.size(), 1);
  for (int i = 0; i < ret.size(); i++) {
    bool flag = ret[i]->compare(s3) == 0;
    ASSERT_EQ(flag, true);
  }
  for (int i = 0; i < ret.size(); i++) {
    delete ret[i];
  }
  ret.clear();

  link.GetLinksReverse(s3, ret);
  ASSERT_EQ(ret.size(), 0);
}

TEST(SortedVectorTest, Simple) { 
  SortedVector<int> sv;
  sv.insert(1);
  sv.insert(0);
  sv.insert(10);
  sv.insert(6);
  sv.insert(7);
  sv.insert(8);
  sv.insert(5);
  sv.insert(9);
  sv.insert(3);
  sv.insert(4);
  sv.insert(2);
  sv.insert(5);
}

TEST(SortedVectorTest, Random1) {
  const int TestNum = 1000;
  vector<int> v;
  v.reserve(TestNum);
  for (int i = 0; i < TestNum; i++) {
    v.push_back(i);
  }
  Random rnd(test::RandomSeed());
  for (int i = 0; i < TestNum - 1; i++) {
    int r = rnd.Uniform(TestNum - i - 1);
    std::swap(v[r], v[TestNum - i - 1]);
  }

  SortedVector<int> sv;
  for (int i = 0; i < TestNum; i++) {
    sv.insert(v[i]);
  }
  int i = 0;
  for (SortedVector<int>::iterator ite = sv.begin(); ite != sv.end(); ite++) {
    ASSERT_EQ(*ite, i);    
    i++;
  }
}

TEST(SortedVectorTest, Random2) {
  const int TestNum = 1000;
  vector<int> v;
  v.reserve(TestNum);
  for (int i = 0; i < TestNum; i++) {
    v.push_back(i);
  }
  Random rnd(test::RandomSeed());
  for (int i = 0; i < TestNum - 1; i++) {
    int r = rnd.Uniform(TestNum - i - 1);
    std::swap(v[r], v[TestNum - i - 1]);
  }

  vector<int> sv;
  for (int i = 0; i < TestNum; i++) {
    sv.push_back(v[i]);
    std::sort(sv.begin(), sv.end());
  }

  int i = 0;
  for (vector<int>::iterator ite = sv.begin(); ite != sv.end(); ite++) {
    ASSERT_EQ(*ite, i);
    i++;
  }
}

TEST(DBLinkTmtTest, Bench) {
  vector<string> vs;
  vs.reserve(kNumKeys);
  string snum = std::to_string(kNumKeys);
  int len = snum.size();
  TmtDBLink link("testdb", "recersive");
  vector<vector<int>> keyLinksReverse;
  unordered_map<string, void*> link_map;
  for (int i = 0; i < kNumKeys; i++) {
    std::stringstream ss;
    ss << std::setw(len) << std::setfill('0') << i;
  /*  vs.push_back(ss.str());
    vector<int> vr;
    keyLinksReverse.push_back(vr);*/

    //vector<string> vk;
    //link.AddLinks(ss.str(), vk);
    link_map.insert(pair<string, void*>(ss.str(), nullptr));
  }

  
  Random rnd(test::RandomSeed());
  cout << " tree size " << sizeof(TrieTree) << endl;
  cout << " tree size " << sizeof(unordered_map<unsigned char, TrieTree*>)
       << endl;
  
  
  for (int i = 0; i < kNumKeys; i++) {
    vector<string> vk;
    for (int j = 0; j < kNumLinks; j++) {
      int r = rnd.Uniform(kNumKeys);
      //vk.push_back(vs[r]);
      //keyLinksReverse[r].push_back(i);
    }
    //link.AddLinks(vs[i], vk);
  }

 //cout << " trees count " << TrieTree::TreesCount
 //      << endl;

 // cout << " trees count " << link.link_to_.GetSize() << endl;
 //cout << " trees count " << link.link_reverse_.GetSize() << endl;

 while (true)
   ;

  vector<string*> ret;
  for (int i = 0; i < kNumKeys; i++) {
    link.GetLinksReverse(vs[i], ret);
    //ASSERT_EQ(ret.size(), keyLinksReverse[i].size());
    for (int i = 0; i < ret.size(); i++) {
      delete ret[i];
    }
    ret.clear();
  }
}
}  // namespace tomatodb

int main(int argc, char** argv) {

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
