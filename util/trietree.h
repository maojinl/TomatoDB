// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_TRIETREE_H_
#define STORAGE_TOMATODB_TRIETREE_H_

#include <map>
#include "leveldb/slice.h"

namespace leveldb {
class TrieTree {
 private:
  int words;
  int prefixes;
  TrieTree* parent;
  TrieTree** edges;
  std::map<TrieTree*, char> links;
  static const unsigned char TRIETREE_EDGE_SIZE = 256;

 public:
  TrieTree(TrieTree* pParent) : words(0), prefixes(0), parent(pParent) {
    edges = new TrieTree*[TRIETREE_EDGE_SIZE];
  }
  TrieTree() : TrieTree(nullptr) {}

  TrieTree* AddWord(Slice str) {
    if (str.size() == 0) {
      words += 1;
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
        }
        return edges[index]->AddWord(str);
      }
    }
  }

  virtual ~TrieTree() {
    for (int i = 0; i <= TRIETREE_EDGE_SIZE; i++) {
      if (edges[i] != nullptr) {
        delete edges[i];
        edges[i] = nullptr;
      }
    }
  }


  TrieTree* FindWord(Slice str) {
    if (str.size() > 0) {
      char k;
      k = str[0];
      str.remove_prefix(1);
      int index = k;
      return edges[index]->FindWord(str);
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
    return;
  }

  bool IsEmpty() { 
    return words == 0 && prefixes == 0;
  }
};

}  // namespace leveldb

#endif  // STORAGE_TOMATODB_TRIETREE_H_