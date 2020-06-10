// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// WriteBatch holds a collection of updates to apply atomically to a DB.
//
// The updates are applied in the order in which they are added
// to the WriteBatch.  For example, the value of "key" will be "v3"
// after the following batch is written:
//
//    batch.Put("key", "v1");
//    batch.Delete("key");
//    batch.Put("key", "v2");
//    batch.Put("key", "v3");
//
// Multiple threads can invoke const methods on a WriteBatch without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same WriteBatch must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_WRITE_PARAMS_H_
#define STORAGE_LEVELDB_INCLUDE_WRITE_PARAMS_H_

#include <string>

#include "leveldb/export.h"
#include "leveldb/status.h"
#include "leveldb/write_batch.h"
#include "leveldb/options.h"

namespace leveldb {

// Information kept for every waiting writer
struct Writer {
  explicit Writer(port::Mutex* mu)
      : batch(nullptr), sync(false), done(false), cv(mu) {}

  Status status;
  WriteBatch* batch;
  bool sync;
  bool done;
  port::CondVar cv;
};

class LEVELDB_EXPORT WriteParams {
 public:

  friend class DBImpl;
  WriteParams() = default;

  // Intentionally copyable.
  WriteParams(const WriteParams&) = delete;
  WriteParams& operator=(const WriteParams&) = delete;
  WriteParams(DBImpl* pDb) : batch(), options(), writer(&(pDb->mutex_)){};
  ~WriteParams() = default;

  WriteBatch batch;
  WriteOptions options;

  void PreparePutParams(const Slice& key,
                        const Slice& value) {
    batch.Put(key, value);
    writer.batch = &batch;
    writer.sync = options.sync;
    writer.done = false;
  };

  void PrepareDeleteParams(const Slice& key) {
    batch.Delete(key);
    writer.batch = &batch;
    writer.sync = options.sync;
    writer.done = false;
  };

  void ClearParams() {
    batch.Clear();
    writer.done = false;
  }

 private:
  Writer writer;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_WRITE_PARAMS_H_
