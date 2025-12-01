#include "skiplist_db.h"
#include "serialize.h"
#include "core/db_factory.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace ycsbc {

mutex SkipList_DB:: mutex_;

void SkipList_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status SkipList_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) { 
  skiplist::SkipList::Iterator iter(&list);
  iter.Seek(key.c_str());
  if(iter.Valid()){
    string entry(iter.key());
    string k, v;
    DeserializeKeyValue(&k, &v, entry);
    assert(k == key);
    if(fields != nullptr){
      DeserializeRowFilter(&result, v, *fields);
    }else{
      DeserializeRow(&result, v, fieldcount_);
    }
    return kOK;
    printf("found\n");
  }else{
    return kNotFound;
  }
}

DB::Status SkipList_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  skiplist::SkipList::Iterator iter(&list);
  iter.Seek(key.c_str());
  for (int i = 0; iter.Valid() && i < len; ++i, iter.Next()){
    string entry(iter.key());
    string k, v;
    DeserializeKeyValue(&k, &v, entry);
    result.push_back(vector<Field>());
    vector<Field> &values = result.back();
    if (fields != nullptr) {
      DeserializeRowFilter(&values, v, *fields);
    } else {
      DeserializeRow(&values, v, fieldcount_);
    }
  }
  return kOK;
}

DB::Status SkipList_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  return kNotImplemented;
}

DB::Status SkipList_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeKeyValue(key, values, &data);
  char* entry = (char*)malloc(sizeof(char)*data.size()+1);
  entry[data.size()] = 0;
  memcpy(entry, data.c_str(), data.size());
  list.Insert(entry);
  return kOK;
}

DB::Status SkipList_DB::Delete(const std::string &table, const std::string &key) {
  return kNotImplemented;
}

DB *NewSkipList_DB() {
  return new SkipList_DB;
}

const bool registered = DBFactory::RegisterDB("skiplist", NewSkipList_DB);

} // ycsbc
