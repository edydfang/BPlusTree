// Copyright [2020] <Copyright Owner>  [legal/copyright]
#include <predefined.h>
#include <string.h>
#include <util/benchmark_utils.h>

#include <cstdio>

vector<uint32_t> date2keyarr(const char* date_str, uint32_t record_id) {
  // 0:commit date 1:ship date 2:receipt date in one int:
  // 31 .. 23 \ 22 ... 9 \ 8 ... 5 \ 4 ... 0
  //  padding \  year    \   month \  day
  char tmp[11];
  vector<uint32_t> comp_key(4);
  int year, month, day;
  const int index[3] = {0, 11, 22};
  for (int i = 0; i < 3; i++) {
    comp_key[i] = 0;
    strncpy(tmp, date_str + index[i], 10);
    sscanf(tmp, "%d-%d-%d", &year, &month, &day);
    comp_key[i] = (year << 9) + (month << 5) + (day);
  }
  comp_key[3] = record_id;
  return comp_key;
}

bpt::vec4_t data2key(const char* date_str, uint32_t record_id) {
  return bpt::vec4_t(date2keyarr(date_str, record_id));
}
