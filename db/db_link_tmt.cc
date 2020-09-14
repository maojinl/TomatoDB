// Copyright (c) 2020 The TomatoDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/db.h"
#include "leveldb/db_link_tmt.h"

namespace tomatodb {
  TmtDBLink::~TmtDBLink() {}

  Status TmtDBLink::AddLinks(const string& key, const vector<string>& links) {
    TrieTree* t = link_to_.AddWord(Slice(key));
    for (int i = 0; i < links.size(); i++) {
      TrieTree* rt = link_reverse_.AddWord(Slice(links[i]));
      rt->AddLink(t);
      t->AddLink(rt);
    }
    return Status::OK();
  }

  Status TmtDBLink::RemoveLinks(const string& key) {
    TrieTree* t = link_to_.FindWord(Slice(key));
    if (t != nullptr) {
      TrieTree::LinksIterator ite = t->LinksBegin();
      TrieTree::LinksIterator iteEnd = t->LinksEnd();
      for (; ite != iteEnd; ite++) {
        TrieTree* rt = ite->first;
        rt->RemoveLink(t);
      }
    }
    return Status::OK();
  }
  }  // namespace tomatodb