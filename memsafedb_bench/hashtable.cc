#include "hashtable.h"
#include "serialize.h"
#include "core/db_factory.h"

using namespace std;

namespace ycsbc {

std::mutex Unordered_Map_DB:: mutex_;

void Unordered_Map_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status Unordered_Map_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {

  string data;
  auto it = ht.find(key);
  if(it == ht.end()){
    return kNotFound;
  }
  data = it->second;
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }
  return kOK;
}

DB::Status Unordered_Map_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  return kNotImplemented;
}

DB::Status Unordered_Map_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  auto it = ht.find(key);
  if(it == ht.end()){
    return kNotFound;
  }
  data = it->second;
  vector<Field> current_values;
  DeserializeRow(&current_values, data, fieldcount_);
  for(Field &new_field: values){
    bool found MAYBE_UNUSED = false;
    for(Field &cur_field: current_values){
      if(cur_field.name == new_field.name){
        found = true;
        cur_field.value = new_field.value;
        break;
      }
    }
    assert(found);
  }
  data.clear();
  SerializeRow(current_values, &data);
  ht.insert_or_assign(key, data);
  return kOK;
}

DB::Status Unordered_Map_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeRow(values, &data);
  auto ret = ht.emplace(key, data);
  if(!ret.second){
    throw utils::Exception(string("Unordered_Map Put error"));
  }
  return kOK;
}

DB::Status Unordered_Map_DB::Delete(const std::string &table, const std::string &key) {
  ht.erase(key);
  return kOK;
}

DB *NewUnordered_Map_DB() {
  return new Unordered_Map_DB;
}

const bool registered_UM = DBFactory::RegisterDB("unordered_map", NewUnordered_Map_DB);

std::mutex ChainedHT_DB:: mutex_;

void ChainedHT_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  if(!init){
    ht.init(stoul(props.GetProperty(CoreWorkload::RECORD_COUNT_PROPERTY)));
  }
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status ChainedHT_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {

  string data = ht.get(key);
  if(data == string()){
    return kNotFound;
  }
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }
  return kOK;
}

DB::Status ChainedHT_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  return kNotImplemented;
}

DB::Status ChainedHT_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  return kNotImplemented;
}

DB::Status ChainedHT_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeRow(values, &data);
  ht.insert(key, data);
  return kOK;
}

DB::Status ChainedHT_DB::Delete(const std::string &table, const std::string &key) {
  ht.remove(key);
  return kOK;
}

DB *NewChainedHT_DB() {
  return new ChainedHT_DB;
}

const bool registered_C = DBFactory::RegisterDB("C_HT", NewChainedHT_DB);

std::mutex OpenAddressingHT_DB:: mutex_;

void OpenAddressingHT_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  if(!init){
    ht.init(stoul(props.GetProperty(CoreWorkload::RECORD_COUNT_PROPERTY)));
  }
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
}

DB::Status OpenAddressingHT_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {

  string data = ht.get(key);
  if(data == string()){
    return kNotFound;
  }
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }
  return kOK;
}

DB::Status OpenAddressingHT_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  return kNotImplemented;
}

DB::Status OpenAddressingHT_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  return kNotImplemented;
}

DB::Status OpenAddressingHT_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  SerializeRow(values, &data);
  ht.insert(key, data);
  return kOK;
}

DB::Status OpenAddressingHT_DB::Delete(const std::string &table, const std::string &key) {
  ht.remove(key);
  return kOK;
}

DB *NewOpenAddressingHT_DB() {
  return new OpenAddressingHT_DB;
}

const bool registered_OA = DBFactory::RegisterDB("OA_HT", NewOpenAddressingHT_DB);

} // ycsbc
