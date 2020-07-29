// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/db_impl_tmt.h"
#include "util/mutexlock.h"
#include "db/version_set.h"
#include "db/write_batch_internal.h"

using namespace leveldb;

namespace tomatodb {

TmtDBImpl::TmtDBImpl(const Options& raw_options, const std::string& dbname)
    : DBImpl(raw_options, dbname) {}

TmtDBImpl::~TmtDBImpl() {}

void TmtDBImpl::InitializeWritersPool(int threads) {
  for (int i = 0; i < threads; i++) {
    Writer* wp = new Writer(&mutex_);
    writers.push_back(wp);
  }
  return;
}

Status TmtDBImpl::WriteEx(const WriteOptions& options, WriteBatch* updates, int tID) {
  Writer* wp = writers[tID];
  wp->batch = updates;
  wp->sync = options.sync;
  wp->done = false;

  MutexLock l(&mutex_);
  writers_.push_back(wp);
  while (!wp->done && wp != writers_.front()) {
    wp->cv.Wait();
  }
  if (wp->done) {

    return wp->status;
  }

  // May temporarily unlock and wait.
  Status status = MakeRoomForWrite(updates == nullptr);
  uint64_t last_sequence = versions_->LastSequence();
  Writer* last_writer = wp;
  if (status.ok() && updates != nullptr) {  // nullptr batch is for compactions
    WriteBatch* write_batch = BuildBatchGroup(&last_writer);
    WriteBatchInternal::SetSequence(write_batch, last_sequence + 1);
    last_sequence += WriteBatchInternal::Count(write_batch);

    // Add to log and apply to memtable.  We can release the lock
    // during this phase since &w is currently responsible for logging
    // and protects against concurrent loggers and concurrent writes
    // into mem_.
    {
      mutex_.Unlock();
      status = log_->AddRecord(WriteBatchInternal::Contents(write_batch));
      bool sync_error = false;
      if (status.ok() && options.sync) {
        status = logfile_->Sync();
        if (!status.ok()) {
          sync_error = true;
        }
      }
      if (status.ok()) {
        status = WriteBatchInternal::InsertInto(write_batch, mem_);
      }
      mutex_.Lock();
      if (sync_error) {
        // The state of the log file is indeterminate: the log record we
        // just added may or may not show up when the DB is re-opened.
        // So we force the DB into a mode where all future writes fail.
        RecordBackgroundError(status);
      }
    }
    if (write_batch == tmp_batch_) tmp_batch_->Clear();

    versions_->SetLastSequence(last_sequence);
  }

  while (true) {
    Writer* ready = writers_.front();
    writers_.pop_front();
    if (ready != wp) {
      ready->status = status;
      ready->done = true;
      ready->cv.Signal();
    }
    if (ready == last_writer) break;
  }

  // Notify new head of write queue
  if (!writers_.empty()) {
    writers_.front()->cv.Signal();
  }
  return status;
}

Status TmtDBImpl::Open(int threads, const Options& options,
                       const std::string& dbname,
                       DB** dbptr) {
  TmtDBImpl* p = new TmtDBImpl(options, dbname);
  p->InitializeWritersPool(threads);
  *dbptr = p;
  return OpenDBCore(options, dbname, dbptr);
}

}  // namespace leveldb
