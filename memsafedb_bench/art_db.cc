#include "art_db.h"
#include "serialize.h"
#include "core/db_factory.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace ycsbc {

mutex ART_DB:: mutex_;

void ART_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status ART_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {
  string data = tree.get(key.c_str());
  if(data == string()){
    return kNotFound;
  }
  /*if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }*/
  return kOK;
}

DB::Status ART_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  auto it = tree.begin(key.c_str());
  auto it_end = tree.end();
  for (int i = 0; it != it_end && i < len; ++i, ++it) {
    string data = *it;
    /*result.push_back(vector<Field>());
    vector<Field> &values = result.back();
    if (fields != nullptr) {
      DeserializeRowFilter(&values, data, *fields);
    } else {
      DeserializeRow(&values, data, fieldcount_);
    }*/
  }
  return kOK;
}

DB::Status ART_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  return kNotImplemented;
}

DB::Status ART_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeRow(values, &data);
  string ret = tree.set(key.c_str(), data);
  assert(ret == string()); 
  return kOK;
}

DB::Status ART_DB::Delete(const std::string &table, const std::string &key) {
  auto ret = tree.del(key.c_str());
  if(ret == string()){
    return kNotFound;
  }
  return kOK;
}

DB *NewART_DB() {
  return new ART_DB;
}

const bool registered = DBFactory::RegisterDB("ART", NewART_DB);

} // ycsbc
