#ifndef PREDEFINED_H_
#define PREDEFINED_H_

#include <string.h>
#include <cstdint>
#include <vector>


namespace bpt {

/* predefined B+ info */
#ifndef UNIT_TEST
#define BP_ORDER 20
#else
#define BP_ORDER 4
#endif

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
  operator bool() const { return k[0] || k[1] || k[2] || k[3]; }
};

inline int keycmp(const key_t &a, const key_t &b) {
  for (int i = 0; i < 4; i++) {
    if (a.k[i] == b.k[i]) {
      continue;
    } else {
      return a.k[i] - b.k[i];
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

#endif  // PREDEFINED_H_
