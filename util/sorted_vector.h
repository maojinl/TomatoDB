// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_SORTED_VECTOR_H_
#define STORAGE_SORTED_VECTOR_H_

#include <vector>

namespace leveldb {

using namespace std;

template <typename T>
class SortedVector {
 public:
  typedef typename vector<T>::iterator iterator;

 private:
  vector<T> v;

 public:
  SortedVector() {}

  virtual ~SortedVector() {}

  bool find(T data) {
    int idx = 0;
    return binarySearch(0, v.size() - 1; data, idx);
  }


  bool insert(T data) {
    if (v.size() == 0) {
      v.push_back(data);
      return true;
    }
    int idx = 0;
    bool found = binarySearch(0, v.size() - 1, data, idx);
    if (found) {
      return false;
    }
    int originSize = v.size();
    if (idx == originSize - 1) {
      v.push_back(data);
    } else {
      v.push_back(v[v.size() - 1]);
      if (idx == originSize - 2) {
        v[v.size() - 2] = data;
      } else {
        int n = v.size() - 2 - idx;
        memmove(&v[idx + 2], &v[idx + 1], n * sizeof(T));
        v[idx + 1] = data;
      }
    }
  }

  iterator begin() { 
    return v.begin();
  }

  iterator end() { 
    return v.end(); 
  }

  void clear() { 
    v.clear(); 
  }

  size_t size() { 
    return v.size();
  }

 private:
  bool binarySearch(int l, int r, T data, int& idx) {
    while (true) {
      if (r <= l) {
        idx = l;
        if (v[l] == data) {
          return true;
        } else if (v[l] > data) {
          idx = l - 1;
          return false;
        } else if (v[r] < data) {
          idx = r;
          return false;
        }
      }

      int m = (l + r) / 2;
      if (v[m] == data) {
        idx = m;
        return true;
      }

      if (v[m] < data) {
        l = m + 1;
      } else {
        r = m - 1;
      }
    }
  }
  
};  // namespace leveldb

}  // namespace leveldb
#endif  // STORAGE_TOMATODB_TRIETREE_H_