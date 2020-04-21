// Copyright [2020] <Copyright Owner>  [legal/copyright]
#ifndef UTIL_BENCHMARK_UTILS_H_
#define UTIL_BENCHMARK_UTILS_H_
#include <vector>
#include <cstdint>
#include "../predefined.h"

using std::vector;

vector<uint32_t> date2keyarr(const char* date_str, uint32_t record_id);
bpt::vec4_t data2key(const char* date_str, uint32_t record_id);

#endif  // UTIL_BENCHMARK_UTILS_H_
