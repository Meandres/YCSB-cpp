#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "../core/db.h"
#include <string>
#include <vector>
#include <span>

void SerializeKeyValue(const std::string &key, const std::vector<ycsbc::DB::Field> &values, std::string *data);
void DeserializeKeyValue(std::string *key, std::string *values, const std::string &data);
void SerializeRow(const std::vector<ycsbc::DB::Field> &values, std::string *data);
void DeserializeRowFilter(std::vector<ycsbc::DB::Field> *values, const std::string &data,
                                     const std::vector<std::string> &fields);
void DeserializeRow(std::vector<ycsbc::DB::Field> *values, const std::string &data, size_t fieldcount);

#endif
