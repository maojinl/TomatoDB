// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_DB_LINK_H_
#define STORAGE_TOMATODB_DB_LINK_H_

#include <vector>
#include <string>
#include "util/trietree.h"
#include "leveldb/status.h"

using namespace leveldb;
using namespace std;

namespace tomatodb {

class TmtDBLink {
 protected:
  TrieTree link_to_;
  TrieTree link_reverse_;
  string db_name_;
  string link_name_;
 public:
  TmtDBLink(const string& dbName, const string& linkName)
      : db_name_(dbName), link_name_(linkName) {}
  virtual ~TmtDBLink();
  TmtDBLink(const TmtDBLink&) = delete;
  TmtDBLink& operator=(const TmtDBLink&) = delete;

  Status AddLinks(const string& key, const vector<string>& links); 
  Status RemoveLinks(const string& key);
  Status GetLinks(const string& key, vector<string*>& links);
  Status GetLinksReverse(const string& key, vector<string*>& links_reverse);
  /*  ~TmtDBImpl() override;

  void InitializeWritersPool(int threads);
  Status WriteEx(const WriteOptions& options, WriteBatch* updates, int tID);*/
};
}  // namespace tomatodb

#endif  // STORAGE_TOMATODB_DB_LINK_H_
