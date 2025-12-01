#include "link_list_db.h"
#include "serialize.h"
#include "core/db_factory.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace ycsbc {

mutex LinkList_DB:: mutex_;

void LinkList_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status LinkList_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) { 
  for(string s: list){
    if(s.starts_with(key)){
      string k, v;
      DeserializeKeyValue(&k, &v, s);
      if(fields != nullptr){
        DeserializeRowFilter(&result, v, *fields);
      }else{
        DeserializeRow(&result, v, fieldcount_);
      }
      return kOK;
    }
  }
  return kNotFound;
}

DB::Status LinkList_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  bool found = false;
  int count = 0;
  for(string s: list){
    if(s.starts_with(key)){
      found = true;
    }
    if(found){
      string k, v;
      DeserializeKeyValue(&k, &v, s);
      result.push_back(vector<Field>());
      vector<Field> &values = result.back();
      if(fields != nullptr){
        DeserializeRowFilter(&values, v, *fields);
      }else{
        DeserializeRow(&values, v, fieldcount_);
      }
      count++;
    }
    if(count >= len){
      break;
    }
  }
  return kOK;
}

DB::Status LinkList_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeKeyValue(key, values, &data);
  for(auto it = list.begin(); it != list.end(); ++it){
    string s = *it;
    if(s.starts_with(key)){
      list.emplace(it, data);
      list.erase(it);
      return kOK;
    }
  }
  return kNotFound;
}

DB::Status LinkList_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeKeyValue(key, values, &data);
  list.emplace_back(data);
  return kOK;
}

DB::Status LinkList_DB::Delete(const std::string &table, const std::string &key) {
  for(auto it = list.begin(); it != list.end(); ++it){
    string s = *it;
    if(s.starts_with(key)){
      list.erase(it);
      return kOK;
    }
  }
  return kNotFound;
}

DB *NewLinkList_DB() {
  return new LinkList_DB;
}

const bool registered = DBFactory::RegisterDB("linklist", NewLinkList_DB);

} // ycsbc
