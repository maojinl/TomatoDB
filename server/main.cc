#include <iostream>
#include "leveldb/db.h"
#include "db/db_impl.h"

namespace tomatodb {




}  // namespace tomatodb
using namespace leveldb;
int main(int argc, char** argv) {
  std::string dbname;
  Env* env = Env::Default();
  env->GetTestDirectory(&dbname);
  dbname += "/dbtest";
  dbname = "e:\\tomato_test\\testdb";
  DestroyDB(dbname, Options());

  DB* db = nullptr;
  Options opts;
  opts.create_if_missing = true;
  Status s = DB::Open(opts, dbname, &db);
  if (!s.ok()) {
    std::cout << s.ToString() << std::endl;
  }
  assert(s.ok());
  delete db;
  DestroyDB(dbname, Options());

  return 0;
}