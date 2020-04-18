#ifndef PREDEFINED_H
#define PREDEFINED_H

#include <stdio.h>
#include <string.h>

namespace bpt {

/* predefined B+ info */
#define BP_ORDER 4

/* key/value type */
typedef int value_t;
struct key_t {
  // 0:commit date 1:ship date 2:receipt date
  // in one int:
  // 31 .. 23 \ 22 ... 9 \ 8 ... 5 \ 4 ... 0
  //  padding \  year    \   month \  day
  int k[3];

  key_t(const char *str = "") {
    // init key
    char tmp[11];
    int year, month, day;
    int index[3] = {0, 11, 22};
    for (int i = 0; i < 3; i++) {
      k[i] = 0;
      strncpy(tmp, str + index[i], 10);
      sscanf(tmp, "%d-%d-%d", &year, &month, &day);
      k[i] = (year << 9) + (month << 5) + (day);
    }

    // original constructor method
    // bzero(k, sizeof(k));
    // strcpy(k, str);
  }

  operator bool() const { return (k[0] == 0 && k[1] == 0 && k[2] == 0); }
};

inline int keycmp(const key_t &l, const key_t &r) {
  for (int i = 0; i < 3; i++) {
    if (l.k[i] == r.k[i]) {
      continue;
    } else {
      return l.k[i] - r.k[i];
    }
  }
  return 0;
}

#define OPERATOR_KEYCMP(type)                      \
  bool operator<(const key_t &l, const type &r) {  \
    return keycmp(l, r.key) < 0;                   \
  }                                                \
  bool operator<(const type &l, const key_t &r) {  \
    return keycmp(l.key, r) < 0;                   \
  }                                                \
  bool operator==(const key_t &l, const type &r) { \
    return keycmp(l, r.key) == 0;                  \
  }                                                \
  bool operator==(const type &l, const key_t &r) { \
    return keycmp(l.key, r) == 0;                  \
  }

}  // namespace bpt

#endif /* end of PREDEFINED_H */
