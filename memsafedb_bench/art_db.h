//
//  art.h
//

#ifndef MEMSAFEDB_ART_DB_H_
#define MEMSAFEDB_ART_DB_H_

#include <core/db.h>
#include "art.hpp"
#include "utils/properties.h"

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>

namespace ycsbc {

class ART_DB : public DB {
 public:
  ART_DB(){}

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
  art::art<std::string> tree;
  int fieldcount_;
  
};

DB *NewART_DB();

} // ycsbc

#endif // MEMSAFEDB_ART_DB_H_

