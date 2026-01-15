#include "serialize.h"
#include <cstdint>
#include <profiler.h>

// Serialization:
// [nb_field] [field_size] [name field0]/[field 0]...[name field n]/[field n]
using namespace std;

void SerializeKeyValue(const std::string &key, const std::vector<ycsbc::DB::Field> &values, std::string *data){
  data->append(key);
  data->append("//"); // This works because the key only contains number and letters 
  SerializeRow(values, data);
}
void DeserializeKeyValue(std::string *key, std::string *values, const std::string &data){
  size_t sep = data.find("//");
  key->append(data.substr(0, sep));
  values->append(data.substr(sep+2, data.size()-sep+2));
}

void SerializeRow(const vector<ycsbc::DB::Field> &values, string *data) {
  auto concat_fields = [](string s, ycsbc::DB::Field f){
    return s + f.name + '/' + f.value;
  };
  data->append(to_string(values.size()) + " " + to_string(values[0].value.size()) + " ");
  data->append(accumulate(values.begin(), values.end(), string(), concat_fields));
}

void DeserializeRowFilter(vector<ycsbc::DB::Field> *values, const string &data,
                                     const vector<string> &fields) {
  const char *p = data.data();
  const char *lim = p + data.size();

  size_t first_space = data.find_first_of(' ');
  printf("deserializerowfilter\n");
  int field_nb = stoi(data.substr(0, first_space));
  int field_size = stoi(data.substr(first_space, data.find_first_of(' ', first_space)));
  printf("field_nb: %u, field_size: %u\n", field_nb, field_size);

  vector<string>::const_iterator filter_iter = fields.begin();
  while (p != lim && filter_iter != fields.end()) {
    assert(p < lim);
    uint32_t len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    string field(p, static_cast<const size_t>(len));
    p += len;
    len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    string value(p, static_cast<const size_t>(len));
    p += len;
    if (*filter_iter == field) {
      values->push_back({field, value});
      filter_iter++;
    }
  }
  assert(values->size() == fields.size());
}

void DeserializeRow(vector<ycsbc::DB::Field> *values, const string &data, size_t fieldcount) {
  size_t first_space = data.find_first_of(' ');
  int field_nb = stoi(data.substr(0, first_space));
  first_space++;
  size_t second_space = data.find_first_of(' ', first_space);
  int field_size = stoi(data.substr(first_space, second_space));

  string remaining = data.substr(second_space+1, data.size());
  for (int i=0; i<field_nb; i++){
    size_t w_pos = remaining.find_first_of('/');
    values->push_back({remaining.substr(0, w_pos), remaining.substr(w_pos+1, field_size)});
    if(w_pos+1+field_size < remaining.size())
      remaining = remaining.substr(w_pos+1+field_size);
  }
  assert(values->size() == fieldcount); 
}
