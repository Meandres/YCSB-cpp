#include "clht_db.h"
#include "core/db_factory.h"
#include "serialize.h"
#include <cmath>

using namespace std;
#ifdef __aarch64__
#define _mm_sfence()  __asm__ __volatile__ ("dmb st" ::: "memory")
#define _mm_lfence()  __asm__ __volatile__ ("dmb ld" ::: "memory")
#define _mm_mfence()  __asm__ __volatile__ ("dmb sy" ::: "memory")
#endif
namespace ycsbc {

mutex CLHT_DB:: mutex_;
bool CLHT_DB::initialized = false;
clht * CLHT_DB::ht = NULL;
std::atomic<uint64_t> CLHT_DB::available_pos;
std::vector<std::string> CLHT_DB::entries;

void CLHT_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  _mm_mfence();
  if(!initialized){
    const utils::Properties &props = *props_;
    size_t read = stoul(props.GetProperty(CoreWorkload::RECORD_COUNT_PROPERTY));
    size_t nb_op = pow(2, ceil(log(read)/log(2)));
    ht = clht_create(nb_op); // TODO: check this size
    assert(ht != NULL);
    entries.reserve(nb_op);
    for(size_t i = 0; i < nb_op; i++){
      entries.push_back(string());
    }
    fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
    available_pos.store(1);
    initialized = true;
  }
}

uint64_t CLHT_DB::ExtractKey(const string &key){
  string nb = key.substr(4);
  uint64_t res = stoul(nb);
  return res;
}


DB::Status CLHT_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result){
  string data;
  uint64_t nb_key = ExtractKey(key);
  clht_val_t v = 0;
  int retry_cnt = 0;
  do{
    v = clht_get(ht->ht, nb_key);
  } while(v == 0);
  if(v == 0 && retry_cnt > 1000){
    printf("failed - nb_key: %lu\n", nb_key);
    return kNotFound;
  }
  /*data = entries[v];
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }*/
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
  if(ret == false){
    throw utils::Exception(string("CLHT Put error"));
  }
  //lock_guard<mutex> lock(mutex_);
  //printf("ht: %p, ht->ht: %p\n", ht, ht->ht);
  //clht_print(ht->ht);
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
