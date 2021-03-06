// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef STORAGE_LEVELDB_DB_DB_IMPL_TMT_H_
#define STORAGE_LEVELDB_DB_DB_IMPL_TMT_H_
#include <vector>
#include "db/db_impl.h"
using namespace leveldb;

namespace tomatodb {

class TmtDBImpl : public DBImpl {
 protected:
  std::vector<Writer*> writers;

 public:
  TmtDBImpl(const Options& options, const std::string& dbname);

  TmtDBImpl(const DBImpl&) = delete;
  TmtDBImpl& operator=(const DBImpl&) = delete;

  ~TmtDBImpl() override;

  void InitializeWritersPool(int threads);
  Status WriteEx(const WriteOptions& options, WriteBatch* updates, int tID);
};
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_DB_IMPL_TMT_H_
