#ifndef ZBPT_H_
#define ZBPT_H_

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./bpt.h"
#include "./predefined.h"
// #include "./zpredefined.h"

namespace bpt {

struct index_zmap_t {
  vec4_t key;
  uint32_t bound[2][2];
  off_t child; /* child's offset */
};

struct internal_node_zmap_t {
  typedef index_zmap_t *child_t;
  off_t parent; /* parent node offset */
  off_t next;
  off_t prev;
  size_t n; /* how many children */
  index_zmap_t children[BP_ORDER];
};

/* the encapulated B+ tree */
class bplus_tree_zmap : public bplus_tree<vec4_t> {
 public:
  explicit bplus_tree_zmap(const char *path, bool force_empty = false);
  // /* abstract operations */
  // int search(const vec4_t &key, value_t *value) const;
  int search_single(const vec4_t &key, value_t *values, size_t max,
                    bool *next = NULL, u_int8_t key_idx = 0) const;
  // int search_range(vec4_t *left, const vec4_t &right, value_t *values,
  //                  size_t max, bool *next = NULL) const;
  int search_range_single(vec4_t *left, const vec4_t &right, value_t *values,
                          size_t max, bool *next = NULL,
                          u_int8_t key_idx = 0) const;
  // int remove(const vec4_t &key);
  int insert(const vec4_t &key, value_t value);

 private:
  off_t search_index(const vec4_t &key) const;
};
}  // namespace bpt
#endif  // ZBPT_H_
