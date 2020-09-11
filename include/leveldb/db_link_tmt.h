// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_TOMATODB_DB_LINK_H_
#define STORAGE_TOMATODB_DB_LINK_H_

#include "util/trietree.h"
using namespace leveldb;

namespace tomatodb {

class TmtDBLink {
 protected:
  TrieTree link_to_;
  TrieTree link_reverse_;

 public:
 /* TmtDBLink(const std::string& dbname);

  TmtDBImpl(const DBImpl&) = delete;
  TmtDBImpl& operator=(const DBImpl&) = delete;

  ~TmtDBImpl() override;

  void InitializeWritersPool(int threads);
  Status WriteEx(const WriteOptions& options, WriteBatch* updates, int tID);*/
};
}  // namespace tomatodb

#endif  // STORAGE_TOMATODB_DB_LINK_H_
