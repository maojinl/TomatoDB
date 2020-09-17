// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_TRIETREE_H_
#define STORAGE_TOMATODB_TRIETREE_H_

#include <map>
#include <stack>
#include "leveldb/slice.h"

#define TRIETREE_EDGE_SIZE 256

namespace leveldb {

class TrieTree {
 public:
  typedef typename std::map<TrieTree*, char>::iterator LinksIterator;

 private:
  int edgesCount;
  //int prefixes;
  TrieTree* parent;
  TrieTree** edges;
  std::map<TrieTree*, char> links;

  unsigned char char_;
  int level_;

 public:
  TrieTree(TrieTree* pParent, unsigned char c, int level)
      : edgesCount(0), /*prefixes(0), */parent(pParent), char_(c), level_(level) {
    edges = new TrieTree*[TRIETREE_EDGE_SIZE];
    //memset(edges, null, sizeof(TrieTree*) * TRIETREE_EDGE_SIZE);
    for (int i = 0; i <= TRIETREE_EDGE_SIZE; i++) {
      edges[i] = nullptr;
    }
  }
  TrieTree() : TrieTree(nullptr, 0, 0) {}

  virtual ~TrieTree() {
    for (int i = 0; i <= TRIETREE_EDGE_SIZE; i++) {
      if (edges[i] != nullptr) {
        delete edges[i];
        edges[i] = nullptr;
      }
    }
  }

  //void ReducePrefix() {
  //  assert(prefixes > 0);
  //  prefixes--;
  //}

  TrieTree* GetParent() { return parent; }

  unsigned char GetChar() { return char_; }

  int GetLevel() { return level_; }

  int GetEdgesCount() { return edgesCount; }

  TrieTree* FindWord(Slice& str) {
    if (str.size() > 0) {
      char k = str[0];
      if (edges[k] == nullptr) {
        return nullptr;
      }

      str.remove_prefix(1);
      return edges[k]->FindWord(str);
    }
    return this;
  }

  void AddLink(TrieTree* data) {
    std::map<TrieTree*, char>::iterator ite = links.find(data);
    if (ite == links.end()) {
      links.insert(std::pair<TrieTree*, char>(data, 0));
    }
    return;
  }

  void RemoveLink(TrieTree* data) {
    std::map<TrieTree*, char>::iterator ite = links.find(data);
    if (ite != links.end()) {
      links.erase(ite);
    }
    return;
  }

  bool IsEmpty() {
    return edgesCount == 0 && links.size() == 0;
  }

  bool IsRoot() { return parent == nullptr; }

   LinksIterator LinksBegin() {
    return links.begin();
  }

   LinksIterator LinksEnd() {
    return links.end();
  }

  TrieTree* AddWord(Slice& str) {
    if (str.size() == 0) {
       return this;
     } else {
       if (str.size() > 0) {
         unsigned char k;
         k = str[0];
         str.remove_prefix(1);
         int index = k;
         AddEdge(k);
         return edges[index]->AddWord(str);
       }
     }
   }

  bool RemoveWord(Slice& str) {
    assert(IsRoot());
    RemoveWordCore(str);
  }

  static bool RemoveNode(TrieTree* root, TrieTree*& node) {
    if (!root->IsRoot()) {
      return false;
    }

    TrieTree* t = node;
    TrieTree* p;
    bool removed = false;
    while (t != root) {
      p = t->GetParent();
      if (t->IsEmpty() && p->RemoveEdge(t->GetChar())) {
        removed = true;
        t = p;
      } 
      else {
        break;
      }
    }
    if (removed) {
      node = nullptr;
    }
    return removed;
  }

  bool RemoveEdge(unsigned char c) { 
    if (edges[c] != nullptr) {
      delete edges[c];
      edges[c] = nullptr;
      edgesCount--;
      return true;
    }
    return false;
  }

  void AddEdge(unsigned char c) {
    if (edges[c] == nullptr) {
      edges[c] = new TrieTree(this, c, level_ + 1);
      edgesCount++;
    }
  }

 private:
  bool RemoveWordCore(Slice& str) {
     if (str.size() == 0) {
       return true;
     }
     char k = str[0];
     if (edges[k] == nullptr) {
       return false;
     }
     str.remove_prefix(1);
     bool ret = edges[k]->RemoveWordCore(str);
     if (!ret) {
       return ret;
     }

     RemoveEdge(k);
     return true;
   }
};

}  // namespace leveldb
#endif  // STORAGE_TOMATODB_TRIETREE_H_