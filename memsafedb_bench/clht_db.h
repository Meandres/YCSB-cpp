//
//  clht_DB.h
//

#ifndef MEMSAFEDB_CLHT_DB_H_
#define MEMSAFEDB_CLHT_DB_H_

#include <core/db.h>
#include "clht.h"
#include "utils/properties.h"

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>

namespace ycsbc {

class CLHT_DB : public DB {
 public:
  CLHT_DB() {}

  void Init() override;

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result) override;

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result) override;

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values) override;

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values) override;

  Status Delete(const std::string &table, const std::string &key) override;

 private:
  uint64_t ExtractKey(const std::string &key);

  static std::mutex mutex_;
  static bool initialized;
  static clht* ht;
  static std::atomic<uint64_t> available_pos;
  static std::vector<std::string> entries;
  int fieldcount_;
  
};

DB *NewCLHT_DB();

} // ycsbc

#endif // MEMSAFEDB_UNORDERED_MAP_DB_H_

