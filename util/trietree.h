// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_TRIETREE_H_
#define STORAGE_TOMATODB_TRIETREE_H_

#include <map>
#include "leveldb/slice.h"

namespace leveldb {

class TrieTree {
 public:
  typedef typename std::map<TrieTree*, char>::iterator LinksIterator;

 private:
  int edgesCount;
  int prefixes;
  TrieTree* parent;
  TrieTree** edges;
  std::map<TrieTree*, char> links;
  static const unsigned char TRIETREE_EDGE_SIZE = 256;

 public:
  TrieTree(TrieTree* pParent) : edgesCount(0), prefixes(0), parent(pParent) {
    edges = new TrieTree*[TRIETREE_EDGE_SIZE];
  }
  TrieTree() : TrieTree(nullptr) {}

  virtual ~TrieTree() {
    for (int i = 0; i <= TRIETREE_EDGE_SIZE; i++) {
      if (edges[i] != nullptr) {
        delete edges[i];
        edges[i] = nullptr;
      }
    }
  }

  void ReducePrefix() {
    assert(prefixes > 0);
    prefixes--;
  }

  TrieTree* GetParent() { return parent; }

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
    if (ite != links.end()) {
      links.insert(std::pair<TrieTree*, char>(data, 0));
    }
    return;
  }

  void RemoveLink(TrieTree* data) {
    std::map<TrieTree*, char>::iterator ite = links.find(data);
    if (ite != links.end()) {
      links.erase(ite);
    }
    if (links.size() == 0) {
      
    }
    return;
  }

  bool IsEmpty() {
    return edgesCount == 0 && prefixes == 0 && links.size() == 0;
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
       prefixes += 1;
       if (str.size() > 0) {
         char k;
         k = str[0];
         str.remove_prefix(1);
         int index = k;
         if (edges[index] == nullptr) {
           edges[index] = new TrieTree(this);
           edgesCount++;
         }
         return edges[index]->AddWord(str);
       }
     }
   }

  bool RemoveWord(Slice& str) {
    assert(IsRoot());
    RemoveWordCore(str);
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

     if (edges[k]->IsEmpty()) {
       ReducePrefix();
       delete edges[k];
       edges[k] = nullptr;
       edgesCount--;
     }
     return true;
   }
};

}  // namespace leveldb
#endif  // STORAGE_TOMATODB_TRIETREE_H_