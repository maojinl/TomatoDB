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
        if (rt->IsEmpty()) {
          TrieTree::RemoveNode(&link_reverse_, t);
        }
      }
    }
    return Status::OK();
  }

  Status TmtDBLink::GetLinksReverse(const string& key,
                                    vector<string*>& links_reverse) {
    TrieTree* rt = link_reverse_.FindWord(Slice(key));
    if (rt != nullptr) {
      TrieTree::LinksIterator ite = rt->LinksBegin();
      TrieTree::LinksIterator iteEnd = rt->LinksEnd();
      for (; ite != iteEnd; ite++) {
        TrieTree* t = ite->first;
        int slen = t->GetLevel();
        string* p_str = new string(slen, char(0));
        while (slen > 0) {
          p_str[slen - 1] = t->GetChar();
          slen--;
          t = t->GetParent();
        }
        links_reverse.push_back(p_str);
      }
    }
    return Status::OK();
  }
}  // namespace tomatodb