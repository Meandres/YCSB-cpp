//
//  various implementations of hashtables
//

#ifndef MEMSAFEDB_HASHTABLE_DB_H_
#define MEMSAFEDB_HASHTABLE_DB_H_

#include <core/db.h>
#include "utils/properties.h"

#include <iostream>
#include <string>
#include <mutex>
#include <unordered_map>
#include <simplehash.hpp>

namespace ycsbc {

class Unordered_Map_DB : public DB {
 public:
  Unordered_Map_DB() {}

  void Init();

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result);

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result);

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Delete(const std::string &table, const std::string &key);

 private:
  std::unordered_map<std::string, std::string> ht;
  static std::mutex mutex_;
  int fieldcount_;
  
};

DB *UnorderedMapDB();

class ChainedHT_DB : public DB {
 public:
  ChainedHT_DB() {}

  void Init();

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result);

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result);

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Delete(const std::string &table, const std::string &key);

 private:
  simplehash::ChainedHashTable<std::string> ht;
  bool init = false;
  static std::mutex mutex_;
  int fieldcount_;
  
};

DB *ChainedHTDB();

class OpenAddressingHT_DB : public DB {
 public:
  OpenAddressingHT_DB() {}

  void Init();

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result);

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result);

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values);

  Status Delete(const std::string &table, const std::string &key);

 private:
  simplehash::OpenAddressingHashTable<std::string> ht;
  bool init = false;
  static std::mutex mutex_;
  int fieldcount_;
  
};

DB *OpenAddressingHTDB();

} // ycsbc

#endif

