#include "./zbpt.h"

#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>

using std::binary_search;
using std::upper_bound;
using std::lower_bound;

namespace bpt {

bplus_tree_zmap::bplus_tree_zmap(const char *path, bool force_empty)
    : bplus_tree(path, force_empty) {}

int bplus_tree_zmap::insert(const vec4_t &key, value_t value) {
  off_t parent = search_index(key);
  off_t offset = search_leaf(parent, key);
  leaf_node_t<vec4_t> leaf;
  map(&leaf, offset);

  // check if we have the same key
  if (binary_search(begin(leaf), end(leaf), key)) return 1;

  if (leaf.n == meta.order) {
    // split when full

    // new sibling leaf
    leaf_node_t<vec4_t> new_leaf;
    node_create(offset, &leaf, &new_leaf);

    // find even split point
    size_t point = leaf.n / 2;
    bool place_right = keycmp(key, leaf.children[point].key) > 0;
    if (place_right) ++point;

    // TODO(edydfang) adjust zone map
    // split
    std::copy(leaf.children + point, leaf.children + leaf.n, new_leaf.children);
    new_leaf.n = leaf.n - point;
    leaf.n = point;

    // which part do we put the key
    if (place_right)
      insert_record_no_split(&new_leaf, key, value);
    else
      insert_record_no_split(&leaf, key, value);

    // save leafs
    unmap(&leaf, offset);
    unmap(&new_leaf, leaf.next);

    // insert new index key
    insert_key_to_index(parent, new_leaf.children[0].key, offset, leaf.next);
  } else {
    insert_record_no_split(&leaf, key, value);
    unmap(&leaf, offset);
  }

  return 0;
}

bool operator<(const vec4_t &l, const index_zmap_t &r) {
  return keycmp(l, r.key) < 0;
}

bool operator<(const index_zmap_t &l, const vec4_t &r) {
  return keycmp(l.key, r) < 0;
}
bool operator==(const vec4_t &l, const index_zmap_t &r) {
  return keycmp(l, r.key) == 0;
}
bool operator==(const index_zmap_t &l, const vec4_t &r) {
  return keycmp(l.key, r) == 0;
}

off_t bplus_tree_zmap::search_index(const vec4_t &key) const {
  off_t org = meta.root_offset;
  int height = meta.height;
  while (height > 1) {
    internal_node_zmap_t node;
    map(&node, org);
    // find the index that is strictly greater than the key
    // why end(node) - 1? we don't care the key of last index, if key is
    // greater than all prior indexes, the last index [end(node)-1] is
    // returned
    index_zmap_t *i = upper_bound(begin(node), end(node) - 1, key);
    org = i->child;
    --height;
  }

  return org;
}

int bplus_tree_zmap::search_range_single(vec4_t *left,
                                                      const vec4_t &right,
                                                      value_t *values,
                                                      size_t max, bool *next,
                                                      u_int8_t key_idx) const {
  int return_code = 0;
  if (key_idx == 0) {
    // range search
    return_code =
        bplus_tree_zmap::search_range(left, right, values, max, next);
  } else {
    // using the zonemap
  }
  return return_code;
}


int bplus_tree_zmap::search_single(const vec4_t &key,
                                                value_t *values, size_t max,
                                                bool *next,
                                                u_int8_t key_idx) const {
  vec4_t left_first;
  vec4_t right_first;
  left_first.k[0] = key.k[0];
  right_first.k[0] = key.k[0];
  right_first.k[1] = right_first.k[2] = right_first.k[3] =
      std::numeric_limits<uint32_t>::max();
  int return_code = bplus_tree<bpt::vec4_t>::search_range_single(
      &left_first, right_first, values, max, next, key_idx);
  return return_code;
}

}  // namespace bpt
