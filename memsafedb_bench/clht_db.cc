#include "clht_db.h"
#include "core/db_factory.h"
#include "serialize.h"
#include <thread>
#include <cmath>

using namespace std;

namespace ycsbc {

mutex CLHT_DB:: mutex_;

void CLHT_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  size_t read = stoul(props.GetProperty(CoreWorkload::RECORD_COUNT_PROPERTY));
  size_t nb_op = pow(2, ceil(log(read)/log(2)));
  if(ht == NULL){
    ht = clht_create(nb_op); // TODO: check this size
    assert(ht != NULL);
    entries.reserve(nb_op*2);
  }
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
  available_pos.store(0);
}

uint64_t CLHT_DB::ExtractKey(const string &key){
  string nb = key.substr(4);
  uint64_t res = stoul(nb);
  return res;
}

DB::Status CLHT_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {

  string data;
  uint64_t nb_key = ExtractKey(key);
  clht_val_t v = clht_get(ht->ht, nb_key);
  if(v == 0){
    return kNotFound;
  }
  data = entries[v];
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }
  return kOK;
}

DB::Status CLHT_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  return kNotImplemented;
}

DB::Status CLHT_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  return kNotImplemented;
}

DB::Status CLHT_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  string data;
  uint64_t nb_key = ExtractKey(key);
  SerializeRow(values, &data);
  uint64_t pos = available_pos.fetch_add(1);
  entries[pos] = data;
  int ret = clht_put(ht, nb_key, pos);
  if(!ret){
    throw utils::Exception(string("CLHT Put error"));
  }
  return kOK;
}

DB::Status CLHT_DB::Delete(const std::string &table, const std::string &key) {
  string data;
  uint64_t nb_key = ExtractKey(key);
  clht_val_t pos = clht_remove(ht, nb_key);
  if(pos == 0){
    return kNotFound;
  }
  entries[pos] = string();
  return kOK;
}

DB *NewCLHT_DB() {
  return new CLHT_DB;
}

const bool registered = DBFactory::RegisterDB("CLHT", NewCLHT_DB);

} // ycsbc
