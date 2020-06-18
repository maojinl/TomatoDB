// Copyright (c) 2011 The JSF Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <atomic>
#include "gtest/gtest.h"
#include "jsf_server.h"

namespace jetserverfoundation {

class JSFServerTest : public testing::Test {
 public:

  JSFServer* server;
  JSFServerTest() { 
	server = new JSFServer();
  }

  ~JSFServerTest() {

  }

 
};

TEST_F(JSFServerTest, Empty) {
  do {

  } while (false);
}

}  // namespace jetserverfoundation

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
