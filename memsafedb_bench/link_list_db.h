//
//  link_list.h
//

#ifndef MEMSAFEDB_LINK_LIST_DB_H_
#define MEMSAFEDB_LINK_LIST_DB_H_

#include <core/db.h>
#include "utils/properties.h"
#include <list>

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>

namespace ycsbc {

class LinkList_DB : public DB {
 public:
  LinkList_DB(){}

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
  std::list<std::string> list;
  int fieldcount_;
  
};

DB *NewLinkList_DB();

} // ycsbc

#endif // MEMSAFEDB_ART_DB_H_

