#include "./zbpt.h"

#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>

using std::binary_search;

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

    // TODO adjust zone map
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

}  // namespace bpt
