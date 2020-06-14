// Copyright (c) 2011 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/db_impl.h"
#include "util/mutexlock.h"
#include "db/version_set.h"
#include "db/write_batch_internal.h"


namespace leveldb {
Status DBImpl::Put(WriteParams& p, const Slice& key, const Slice& val) {
  p.ClearParams();
  p.PreparePutParams(key, val);
  return Write(p);
}

void DBImpl::AppendPut(WriteParams& p, const Slice& key, const Slice& val) {
  p.PreparePutParams(key, val);
}

Status DBImpl::Delete(WriteParams& p, const Slice& key) {
  p.ClearParams();
  p.PrepareDeleteParams(key);
  return Write(p);
}

void DBImpl::AppendDelete(WriteParams& p, const Slice& key) {
  p.PrepareDeleteParams(key);
}

Status DBImpl::Write(WriteParams& p) {
  MutexLock l(&write_worker_mutex_);
  if (writers_.empty()) {
    write_worker_idle_cv_.Signal();
  }
  writers_.push_back(&(p.writer));
  while (!p.writer.done) {
    p.writer.cv.Wait();
  }
  return p.writer.status;
}

void DBImpl::PreWriteWorker() {
  background_polling_thread_runing = true;
  env_->StartThread(&DBImpl::WriteWorkerThread, this);
}

void DBImpl::WriteWorkerThread(void* db) {
  reinterpret_cast<DBImpl*>(db)->WriteWorker();
}

void DBImpl::WriteWorker() {
  // May temporarily unlock and wait.
  while (!shutting_down_.load(std::memory_order_acquire)) {
    write_worker_mutex_.Lock();
    if (!writers_.empty()) {
      write_worker_mutex_.Unlock();
      Writer* w = writers_.front();
      WriteBatch* updates = w->batch;

      mutex_.Lock();
      Status status = MakeRoomForWrite(updates == nullptr);
      mutex_.Unlock();

      uint64_t last_sequence = versions_->LastSequence();
      Writer* last_writer = w;
      if (status.ok() &&
          updates != nullptr) {  // nullptr batch is for compactions

        write_worker_mutex_.Lock();
        WriteBatch* write_batch = BuildBatchGroup(&last_writer);
        write_worker_mutex_.Unlock();

        WriteBatchInternal::SetSequence(write_batch, last_sequence + 1);
        last_sequence += WriteBatchInternal::Count(write_batch);

        // Add to log and apply to memtable.  We can release the lock
        // during this phase since &w is currently responsible for logging
        // and protects against concurrent loggers and concurrent writes
        // into mem_.
        {
          status = log_->AddRecord(WriteBatchInternal::Contents(write_batch));
          bool sync_error = false;
          if (status.ok() && w->sync) {
            status = logfile_->Sync();
            if (!status.ok()) {
              sync_error = true;
            }
          }
          if (status.ok()) {
            status = WriteBatchInternal::InsertInto(write_batch, mem_);
          }

          if (sync_error) {
            // The state of the log file is indeterminate: the log record we
            // just added may or may not show up when the DB is re-opened.
            // So we force the DB into a mode where all future writes fail.
            mutex_.Lock();
            RecordBackgroundError(status);
            mutex_.Unlock();
          }
        }
        if (write_batch == tmp_batch_) tmp_batch_->Clear();
        versions_->SetLastSequence(last_sequence);
      }

      write_worker_mutex_.Lock();
      while (!shutting_down_.load(std::memory_order_acquire)) {
        Writer* ready = writers_.front();
        writers_.pop_front();
        ready->status = status;
        ready->done = true;
        ready->cv.Signal();
        if (ready == last_writer) break;
      }
      write_worker_mutex_.Unlock();
    } else {
      // env_->SleepForMicroseconds(10);
      write_worker_idle_cv_.Wait();
      write_worker_mutex_.Unlock();
    }
  }
  background_polling_thread_runing = false;
  return;
}

Status DBImpl::TEST_CompactMemTableEx() {
  WriteParams* p = CreateParams();
  p->writer.batch = nullptr;
  Status s = Write(*p);
  if (s.ok()) {
    // Wait until the compaction completes
    MutexLock l(&mutex_);
    while (imm_ != nullptr && bg_error_.ok()) {
      background_work_finished_signal_.Wait();
    }
    if (imm_ != nullptr) {
      s = bg_error_;
    }
  }
  delete p;
  return s;
}

}  // namespace leveldb
