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

  bool find(T data, int& idx) {}

  bool insert(T data) {}

 private:
  bool binarySearch(int l, int r, T data, int& idx) {
    if (v[l] > data) {
      idx = l - 1;
      return false;
    } else if (v[r] < data) {
      idx = r;
      return false;
    }
    while (true) {
      if (r <= l) {
        idx = l;
        if (v[l] == data) {
          return true;
        } else {
          return false;
        }
      }

      int m = (l + r) / 2;
      if (v[m] == data) {
        idx = m;
        return true;
      }

      if (v[m] < data) {
        l = m;
      } else {
        r = m;
      }
    }
  }
  
};  // namespace leveldb

}  // namespace leveldb
#endif  // STORAGE_TOMATODB_TRIETREE_H_