// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_TRIETREE_H_
#define STORAGE_TOMATODB_TRIETREE_H_

#include <map>
#include <stack>
#include <unordered_map>

#include "leveldb/slice.h"
#include "sorted_vector.h"

namespace leveldb {

using namespace std;
class TrieTree;

class TrieTreeEdgeNode {
 public:
  unsigned char edgeChar;
  TrieTree* edge;
  TrieTreeEdgeNode(unsigned char c, TrieTree* t) : edgeChar(c), edge(t) {}
};


class TrieTreeEdgeNodeComp {
 public:
  bool operator()(const TrieTreeEdgeNode* a, const TrieTreeEdgeNode* b) {
    return a->edgeChar < b->edgeChar;
  }
};

class TrieTree {
 public:
  typedef typename SortedVector<TrieTree*>::iterator LinksIterator;
  typedef
      typename SortedVector<TrieTreeEdgeNode*, TrieTreeEdgeNodeComp>::iterator
      EdgesIterator;
 private:
  //int prefixes;
  TrieTree* parent;
  //TrieTree** edges;
  //unordered_map<unsigned char, TrieTree*> edges;
  SortedVector<TrieTreeEdgeNode*, TrieTreeEdgeNodeComp>* edges;
  //vector<TrieTree*> edges;
  //vector<unsigned char> edgesChar;
  //SortedVector<TrieTree*> links;

  unsigned char char_;
  size_t level_;

 public:
  static int TreesCount;
  TrieTree(TrieTree* pParent, unsigned char c, int level)
      : /*prefixes(0), */ parent(pParent), char_(c), level_(level), edges(nullptr) {
    //edges = new TrieTree*[TRIETREE_EDGE_SIZE];
    //for (int i = 0; i <= TRIETREE_EDGE_SIZE; i++) {
    //  edges[i] = nullptr;
    //}
    TrieTree::TreesCount++;
  }
  TrieTree() : TrieTree(nullptr, 0, 0) {}

  virtual ~TrieTree() {
    //unordered_map<unsigned char, TrieTree*>::iterator ite = edges.begin();
    if (edges != nullptr) {
      EdgesIterator ite = edges->begin();
      for (; ite != edges->end(); ite++) {
        delete *ite;
      }
      edges->clear();
      delete edges;
      edges = nullptr;
    }
  }

  //void ReducePrefix() {
  //  assert(prefixes > 0);
  //  prefixes--;
  //}

  TrieTree* GetParent() { return parent; }

  unsigned char GetChar() { return char_; }

  size_t GetLevel() { return level_; }

  size_t GetEdgesCount() { return edges == nullptr ? 0 : edges->size(); }

  size_t GetSize() {
    size_t s = sizeof(*this);
    if (edges != nullptr) {
      EdgesIterator ite = edges->begin();
      for (; ite != edges->end(); ite++) {
        s += (*ite)->edge->GetSize();
      }
    }
    return s;
  }

  TrieTree* FindWord(Slice& str) {
    if (str.size() > 0) {
      char k = str[0];
      str.remove_prefix(1);
      //if (edges[k] == nullptr) {
      //  return nullptr;
      //}
      //return edges[k]->FindWord(str);
      //unordered_map<unsigned char, TrieTree*>::iterator ite = edges.find(k);
      int idx = 0;
      for (; idx < edgesChar.size(); idx++) {
        if (edgesChar[idx] == k) {
          break;
        }
      }

      if (idx == edgesChar.size() || edges[idx] == nullptr) {
        return nullptr;
      }
      return edges[idx]->FindWord(str);
    }
    return this;
  }

  //std::vector<TrieTree*> vt;
  void AddLink(TrieTree* data) {
    //std::map<TrieTree*, char>::iterator ite = links.find(data);
    //if (ite == links.end()) {
    //  links.insert(std::pair<TrieTree*, char>(data, 0));
    //}

    //vt.push_back(data);
    return;
  }

  void RemoveLink(TrieTree* data) {
    //std::map<TrieTree*, char>::iterator ite = links.find(data);
    //if (ite != links.end()) {
    //  links.erase(ite);
    //}
    return;
  }

  void RemoveAllLink() {
    //links.clear();
    return;
  }

  bool IsEmpty() {
    //return edgesCount == 0 && links.size() == 0;
    return false;
  }

  bool IsRoot() { return parent == nullptr; }

  LinksIterator LinksBegin() {
    //return links.begin();
    return LinksIterator();
  }

  LinksIterator LinksEnd() {
    //return links.end();
    return LinksIterator();
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
         TrieTree* t = AddEdge(k);
         return t->AddWord(str);
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
    //if (edges[c] != nullptr) {
    //  delete edges[c];
    //  edges[c] = nullptr;
    //  edgesCount--;
    //  return true;
    //}
    //return false;

    //unordered_map<unsigned char, TrieTree*>::iterator ite = edges.find(c);
    //if (ite != edges.end()) {
    //  edges.erase(ite);
    //  return true;
    //}
    int idx = 0;
    for (; idx < edgesChar.size(); idx++) {
      if (edgesChar[idx] == c) {
        break;
      }
    }

    if (idx < edgesChar.size()) {
      edges[idx] == nullptr;
      return true;
    }
    return false;
  }

  TrieTree* AddEdge(unsigned char c) {
    //if (edges[c] == nullptr) {
    //  edges[c] = new TrieTree(this, c, level_ + 1);
    //  edgesCount++;
    //}
    //return edges[c];

    //unordered_map<unsigned char, TrieTree*>::iterator ite = edges.find(c);
    //if (ite == edges.end()) {
    //  TrieTree* t = new TrieTree(this, c, level_ + 1);
    //  edges.insert(pair<unsigned char, TrieTree*>(c, t));
    //  return t;
    //} 
    //else {
    //  return ite->second;
    //}
    int idx = 0;
    for (; idx < edgesChar.size(); idx++) {
      if (edgesChar[idx] == c) {
        break;
      }
    }

    if (idx == edgesChar.size()) {
      TrieTree* t = new TrieTree(this, c, level_ + 1);
      edgesChar.push_back(c);
      edges.push_back(t);
      return t;
    } else {
      return edges[idx];
    }
  }

 private:
  bool RemoveWordCore(Slice& str) {
    if (str.size() == 0) {
      return true;
    }
    char k = str[0];
    str.remove_prefix(1);
    //if (edges[k] == nullptr) {
    //  return false;
    //}
    //bool ret = edges[k]->RemoveWordCore(str);
    //if (!ret) {
    //  return ret;
    //}

    //unordered_map<unsigned char, TrieTree*>::iterator ite = edges.find(k);
    //if (ite == edges.end()) {
    //  return false;
    //}
    //bool ret = ite->second->RemoveWordCore(str);

    int idx = 0;
    for (; idx < edgesChar.size(); idx++) {
      if (edgesChar[idx] == k) {
        break;
      }
    }

    if (idx == edgesChar.size() || edges[idx] == nullptr) {
      return false;
    }
    bool ret = edges[idx]->RemoveWordCore(str);

    RemoveEdge(k);
    return true;
  }
};

}  // namespace leveldb
#endif  // STORAGE_TOMATODB_TRIETREE_H_