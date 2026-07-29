//
//  btree.h
//

#ifndef MEMSAFEDB_BTREE_DB_H_
#define MEMSAFEDB_BTREE_DB_H_

#include <core/db.h>
#include "btree.hpp"
#include "utils/properties.h"

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>

namespace ycsbc {

class BTree_DB : public DB {
 public:
  BTree_DB(){}

  void Init() override;

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result) override;

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result) override;

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values) override;

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values) override;

  Status Delete(const std::string &table, const std::string &key) override;

 private:
  static std::mutex mutex_;
  // Shared across all per-thread BTree_DB instances (YCSB makes one DB per
  // thread) so the thread sweep exercises one concurrent tree, like CLHT's ht.
  static BTree::BTree tree;
  int fieldcount_;
  int len_payload;
  
};

DB *NewBTree_DB();

} // ycsbc

#endif // MEMSAFEDB_BTree_DB_H_

