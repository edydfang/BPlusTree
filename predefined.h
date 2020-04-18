#ifndef PREDEFINED_H
#define PREDEFINED_H

#include <string.h>

namespace bpt {

/* predefined B+ info */
#define BP_ORDER 20

/* key/value type */
typedef int value_t;
struct key_t {
    //0:commit date 1:ship date 2:receipt date
    //in one int:
    // 31 .. 23 \ 22 ... 9 \ 8 ... 5 \ 4 ... 0 
    //  padding \  year    \   month \  day    
    int k[3];

    key_t(const char *str = "")
    {
        //init key
        char tmp[11];
        int year, month, day;
        int index[3] = {0, 11, 22};
        for (int i = 0 ; i < 3 ; i ++){
            k[i] = 0;
            strncpy(tmp, str+index[i], 10);
            sscanf(tmp, "%d-%d-%d", &year, &month, &day);
            k[i] = (year << 9) + (month << 5) + (day);
        }

        // original constructor method
        // bzero(k, sizeof(k));
        // strcpy(k, str);
    }

    operator bool() const {
        // return strcmp(k, "");
        return (k[0] == 0 && k[1] == 0 && k[2] == 0);
    }
};

inline int keycmp(const key_t &a, const key_t &b) {
    // int x = strlen(a.k) - strlen(b.k);
    // return x == 0 ? strcmp(a.k, b.k) : x;
    for (int i = 0 ; i < 3 ; i ++){
        if (a.k[i] == b.k[i]){
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

#endif /* end of PREDEFINED_H */
