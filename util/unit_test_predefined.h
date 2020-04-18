#ifndef UTIL_UNIT_TEST_PREDEFINED_H_
#define UTIL_UNIT_TEST_PREDEFINED_H_

#include <stdio.h>
#include <string.h>

#include <cstdint>
#include <vector>

namespace bpt {

/* predefined B+ info */
#define BP_ORDER 4

/* key/value type */
typedef int value_t;
struct key_t {
  uint32_t k[4];
  explicit key_t(const std::vector<uint32_t> key_arr) {
    for (size_t i = 0; i < 4; i++) {
      k[i] = key_arr[i];
    }
  }
  key_t() {
    for (size_t i = 0; i < 4; i++) {
      k[i] = 0;
    }
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

#endif  // UTIL_UNIT_TEST_PREDEFINED_H_
