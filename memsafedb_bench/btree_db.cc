#include "btree_db.h"
#include "serialize.h"
#include "core/db_factory.h"
#include <cmath>
#include <cassert>
#include <profiler.h>

using namespace std;

namespace ycsbc {

mutex BTree_DB:: mutex_;

void BTree_DB::Init() {
  lock_guard<mutex> lock(mutex_);
  const utils::Properties &props = *props_;
  fieldcount_ = stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY, CoreWorkload::FIELD_COUNT_DEFAULT));
  int fieldlength_ = stoi(props.GetProperty(CoreWorkload::FIELD_LENGTH_PROPERTY, CoreWorkload::FIELD_LENGTH_DEFAULT));
  string field_prefix_ = props.GetProperty(CoreWorkload::FIELD_NAME_PREFIX, CoreWorkload::FIELD_NAME_PREFIX_DEFAULT);
  len_payload = fieldcount_ * (fieldlength_ + field_prefix_.size() + to_string(fieldcount_).size()) + 1 /* the slash*/ + to_string(fieldcount_).size() /* the number of fields */ + to_string(fieldlength_).size() /* the size of fields */ + 2 /* two spaces after each count and length */; 
  //uint64_t ops = stoul(props.GetProperty(CoreWorkload::RECORD_COUNT_PROPERTY)) + stoul(props.GetProperty(CoreWorkload::OPERATION_COUNT_PROPERTY));
  uint64_t memsize = 8ull * 1024 * 1024 * 1024;
  if(!tree.initialized){
    tree.init(memsize);
  }
}

DB::Status BTree_DB::Read(const std::string &table, const std::string &key,
                         const std::vector<std::string> *fields, std::vector<Field> &result) {
  vector<uint8_t> k(key.size());
  memcpy(k.data(), key.data(), key.size());
  vector<uint8_t> payload(len_payload);
  if(!tree.lookup({k.data(), k.size()}, [&](span<uint8_t> p) {
    assert(len_payload == p.size());
    memcpy(payload.data(), p.data(), p.size());
  })){
    return kNotFound;
  }
  /*string data(reinterpret_cast<const char*>(payload.data()), payload.size()); 
  if(fields != nullptr){
    DeserializeRowFilter(&result, data, *fields);
  }else{
    DeserializeRow(&result, data, fieldcount_);
  }*/
  return kOK;
}

DB::Status BTree_DB::Scan(const std::string &table, const std::string &key, int len,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<Field>> &result) {
  vector<uint8_t> k(key.size());
  memcpy(k.data(), key.data(), key.size());
  vector<uint8_t> payload(len_payload);
  int i = 0;
  tree.scanAsc({k.data(), k.size()}, [&](BTree::BTreeNode& node, unsigned slot){
    memcpy(payload.data(), node.getPayload(slot).data(), node.getPayload(slot).size());
    /*string data(reinterpret_cast<const char*>(payload.data()), payload.size()); 
    result.push_back(vector<Field>());
    vector<Field> &values = result.back();
    if (fields != nullptr) {
      DeserializeRowFilter(&values, data, *fields);
    } else {
      DeserializeRow(&values, data, fieldcount_);
    }*/
    if(i>= len)
      return true;
    i++;
    return false;
  });
  return kOK;
}

DB::Status BTree_DB::Update(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  vector<uint8_t> k(key.size());
  memcpy(k.data(), key.data(), key.size());
  string data;
  SerializeRow(values, &data);
  vector<uint8_t> new_payload(data.size());
  memcpy(new_payload.data(), data.data(), data.size());
  if(!tree.updateInPlace({k.data(), k.size()}, [&](span<uint8_t> old_payload) {
    memcpy(old_payload.data(), new_payload.data(), new_payload.size());
  })){
    return kNotFound;
  }
  return kOK;
}

DB::Status BTree_DB::Insert(const std::string &table, const std::string &key,
                           std::vector<Field> &values) {
  vector<uint8_t> k(key.size());
  memcpy(k.data(), key.data(), key.size());
  string data;
  SerializeRow(values, &data);
  vector<uint8_t> new_payload(data.size());
  assert(data.size() == len_payload);
  memcpy(new_payload.data(), data.data(), data.size());
  tree.insert({k.data(), k.size()}, {new_payload.data(), new_payload.size()});
  return kOK;
}

DB::Status BTree_DB::Delete(const std::string &table, const std::string &key) {
  vector<uint8_t> k(key.size());
  memcpy(k.data(), key.data(), key.size());
  tree.remove({k.data(), k.size()});
  return kOK;
}

DB *NewBTree_DB() {
  return new BTree_DB;
}

const bool registered = DBFactory::RegisterDB("btree", NewBTree_DB);

} // ycsbc
