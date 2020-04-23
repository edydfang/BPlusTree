#ifndef ZBPT_H_
#define ZBPT_H_

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue>
#include <tuple>

#include "./bpt.h"
#include "./predefined.h"

#define SIZE_NO_CHILDREN_ZMAP \
  sizeof(leaf_node_t<vec4_t>) - BP_ORDER * sizeof(record_t<vec4_t>)

using std::binary_search;
using std::lower_bound;
using std::numeric_limits;
using std::tuple;
using std::upper_bound;

namespace bpt {

inline void copy_arr(uint32_t *bounds1d, uint32_t bounds2d[][2]);

struct index_zmap_t {
  vec4_t key;
  off_t child; /* child's offset */
  uint32_t bound[2][2];
};

struct internal_node_zmap_t {
  typedef index_zmap_t *child_t;
  bool node_type = 0;  // label as internal
  off_t parent;        /* parent node offset */
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
  int search_range_single(vec4_t *left, const vec4_t &right, value_t *values,
                          size_t max, bool *next = NULL, u_int8_t key_idx = 0,
                          std::queue<tuple<off_t, int>> *state_queue = NULL,
                          int* start_pos_in_leaf = 0) const;
  int search_single(vec4_t &key, value_t *values, size_t max, bool *next = NULL,
                    u_int8_t key_idx = 0,
                    std::queue<tuple<off_t, int>> *state_queue = NULL,
                    int* start_pos_in_leaf = 0) const;
  /* insert into leaf without split */
  void insert_record_no_split(leaf_node_t<vec4_t> *leaf, const vec4_t &key,
                              const value_t &value);
  int insert(const vec4_t &key, value_t value);

 private:
  off_t search_index(const vec4_t &key) const;
  /* change children's parent */
  void reset_index_children_parent(index_zmap_t *begin, index_zmap_t *end,
                                   off_t parent);
  /* add key to the internal node */
  void insert_key_to_index(off_t offset, const vec4_t &key, off_t value,
                           off_t after, uint32_t old_bounds[4],
                           uint32_t after_bounds[4]);
  void insert_key_to_index_no_split(internal_node_zmap_t &node,
                                    const vec4_t &key, off_t value);
  void get_leaf_bounds(const leaf_node_t<vec4_t> &leaf, uint32_t bounds[4]);
  void get_internal_bounds(const internal_node_zmap_t &node,
                           uint32_t bounds[4]);
  void set_bounds(index_zmap_t *index_p, uint32_t bounds_arr[4]);
  void node_create(off_t offset, internal_node_zmap_t *node,
                   internal_node_zmap_t *next);
  off_t alloc(internal_node_zmap_t *node) {
    node->n = 1;
    meta.internal_node_num++;
    return bplus_tree<vec4_t>::alloc(sizeof(internal_node_zmap_t));
  }
};
}  // namespace bpt
#endif  // ZBPT_H_
