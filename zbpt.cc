#include "./zbpt.h"

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>

using std::binary_search;
using std::lower_bound;
using std::numeric_limits;
using std::upper_bound;

namespace bpt {

bplus_tree_zmap::bplus_tree_zmap(const char *path, bool force_empty)
    : bplus_tree(path, force_empty) {}

void bplus_tree_zmap::insert_record_no_split(leaf_node_t<vec4_t> *leaf,
                                             const vec4_t &key,
                                             const value_t &value) {
  record_t<vec4_t> *where = upper_bound(begin(*leaf), end(*leaf), key);
  std::copy_backward(where, end(*leaf), end(*leaf) + 1);

  where->key = key;
  where->value = value;
  leaf->n++;
}

void bplus_tree_zmap::get_leaf_bounds(const leaf_node_t<vec4_t> &leaf,
                                      uint32_t bounds[4]) {
  // bounds 0: min for col2, 1: max for col2, 2: min for col3, 3: max for col3
  assert(leaf.node_type == 1);
  const record_t<vec4_t> *cur, *end;
  bounds[1] = bounds[3] = 0;
  bounds[0] = bounds[2] = numeric_limits<uint32_t>::max();
  cur = leaf.children;
  end = leaf.children + leaf.n;
  while (cur != end) {
    bounds[0] = std::min({bounds[0], cur->key.k[1]});
    bounds[1] = std::max({bounds[1], cur->key.k[1]});
    bounds[2] = std::min({bounds[2], cur->key.k[2]});
    bounds[3] = std::max({bounds[3], cur->key.k[2]});
    cur++;
  }
  return;
}

void bplus_tree_zmap::get_internal_bounds(const internal_node_zmap_t &node,
                                          uint32_t bounds[4]) {
  assert(node.node_type == 0);
  // bounds 0: min for col2, 1: max for col2, 2: min for col3, 3: max for col3
  const index_zmap_t *cur, *end;
  bounds[1] = bounds[3] = 0;
  bounds[0] = bounds[2] = numeric_limits<uint32_t>::max();
  cur = node.children;
  end = node.children + node.n;
  while (cur != end) {
    bounds[0] = std::min({bounds[0], cur->bound[0][0]});
    bounds[1] = std::max({bounds[1], cur->bound[0][1]});
    bounds[2] = std::min({bounds[2], cur->bound[1][0]});
    bounds[3] = std::max({bounds[3], cur->bound[1][1]});
    cur++;
  }
  return;
}

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
    bplus_tree<vec4_t>::node_create(offset, &leaf, &new_leaf);

    // find even split point
    size_t point = leaf.n / 2;
    bool place_right = keycmp(key, leaf.children[point].key) > 0;
    if (place_right) ++point;

    // split
    std::copy(leaf.children + point, leaf.children + leaf.n, new_leaf.children);
    new_leaf.n = leaf.n - point;
    leaf.n = point;

    // which part do we put the key
    if (place_right)
      insert_record_no_split(&new_leaf, key, value);
    else
      insert_record_no_split(&leaf, key, value);
    // scan two leaf to get new range
    uint32_t left_bounds[4], right_bounds[4];
    get_leaf_bounds(leaf, left_bounds);
    get_leaf_bounds(new_leaf, right_bounds);
    // save leafs
    unmap(&leaf, offset);
    unmap(&new_leaf, leaf.next);

    // insert new index key
    insert_key_to_index(parent, new_leaf.children[0].key, offset, leaf.next,
                        left_bounds, right_bounds);
  } else {
    insert_record_no_split(&leaf, key, value);
    unmap(&leaf, offset);
  }

  return 0;
}

void bplus_tree_zmap::reset_index_children_parent(index_zmap_t *begin,
                                                  index_zmap_t *end,
                                                  off_t parent) {
  // this function can change both internal_node_t and leaf_node_t's parent
  // field, but we should ensure that:
  // 1. sizeof(internal_node_t) <= sizeof(leaf_node_t)
  // 2. parent field is placed in the beginning and have same size
  internal_node_zmap_t node;
  while (begin != end) {
    map(&node, begin->child);
    node.parent = parent;
    unmap(&node, begin->child, SIZE_NO_CHILDREN_ZMAP);
    ++begin;
  }
}

void bplus_tree_zmap::insert_key_to_index(off_t offset, const vec4_t &key,
                                          off_t old, off_t after,
                                          uint32_t old_bounds[4],
                                          uint32_t after_bounds[4]) {
  // offset: parent internal node
  // old: left leaf/internal node
  // after: right leaf/internal node
  // two bounds for each half of the index
  if (offset == 0) {
    // create new root node
    internal_node_zmap_t root;
    root.next = root.prev = root.parent = 0;
    meta.root_offset = alloc(&root);
    meta.height++;

    // insert `old` and `after`
    root.n = 2;
    root.children[0].key = key;
    root.children[0].child = old;
    root.children[1].child = after;

    // update zone map
    // we need to scan the lower level index or leaf to get new zonemap
    // not the same function for leaf node and the internal node
    std::copy(&old_bounds[0], &old_bounds[0] + 4,
              &(root.children[0].bound[0][0]));
    std::copy(&after_bounds[0], &after_bounds[0] + 4,
              &(root.children[1].bound[0][0]));

    unmap(&meta, OFFSET_META);
    unmap(&root, meta.root_offset);

    // update children's parent
    reset_index_children_parent(begin(root), end(root), meta.root_offset);
    return;
  }

  internal_node_zmap_t node;
  map(&node, offset);
  assert(node.n <= meta.order);

  if (node.n == meta.order) {
    // split when full

    internal_node_zmap_t new_node;
    node_create(offset, &node, &new_node);

    // find even split point
    size_t point = (node.n - 1) / 2;
    bool place_right = keycmp(key, node.children[point].key) > 0;
    if (place_right) ++point;

    // prevent the `key` being the right `middle_key`
    // example: insert 48 into |42|45| 6|  |
    if (place_right && keycmp(key, node.children[point].key) < 0) point--;

    vec4_t middle_key = node.children[point].key;

    // split
    std::copy(begin(node) + point + 1, end(node), begin(new_node));
    new_node.n = node.n - point - 1;
    node.n = point + 1;

    // put the new key
    if (place_right)
      insert_key_to_index_no_split(new_node, key, after);
    else
      insert_key_to_index_no_split(node, key, after);

    // get zonemap for the node and new_node
    uint32_t node_bounds[4], new_node_bounds[4];
    get_internal_bounds(node, node_bounds);
    get_internal_bounds(new_node, new_node_bounds);

    unmap(&node, offset);
    unmap(&new_node, node.next);

    // update children's parent
    reset_index_children_parent(begin(new_node), end(new_node), node.next);

    // recursively update the parent
    // give the middle key to the parent
    // note: middle key's child is reserved
    insert_key_to_index(node.parent, middle_key, offset, node.next, node_bounds,
                        new_node_bounds);
  } else {
    insert_key_to_index_no_split(node, key, after);
    unmap(&node, offset);
  }
}

void bplus_tree_zmap::insert_key_to_index_no_split(internal_node_zmap_t &node,
                                                   const vec4_t &key,
                                                   off_t value) {
  index_zmap_t *where = upper_bound(begin(node), end(node) - 1, key);

  // move later index forward
  std::copy_backward(where, end(node), end(node) + 1);

  // insert this key
  where->key = key;
  where->child = (where + 1)->child;
  (where + 1)->child = value;

  // update the where and where+1 bound here
  // assume they are all internal nodes here
  internal_node_zmap_t tmp_node;
  uint32_t bounds[4];
  map(&tmp_node, where->child);
  // check type first
  if (tmp_node.node_type != 0) {
    // it is leaf node
    leaf_node_t<vec4_t> *leaf = (leaf_node_t<vec4_t> *)&tmp_node;
    get_leaf_bounds(*leaf, bounds);
    std::copy(&bounds[0], &bounds[0] + 3, &(where->bound[0][0]));
    map(leaf, (where + 1)->child);
    get_leaf_bounds(*leaf, bounds);
    std::copy(&bounds[0], &bounds[0] + 3, &((where + 1)->bound[0][0]));
  } else {
    get_internal_bounds(tmp_node, bounds);
    std::copy(&bounds[0], &bounds[0] + 3, &(where->bound[0][0]));
    map(&tmp_node, (where + 1)->child);
    get_internal_bounds(tmp_node, bounds);
    std::copy(&bounds[0], &bounds[0] + 3, &((where + 1)->bound[0][0]));
  }
  node.n++;
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

int bplus_tree_zmap::search_range_single(vec4_t *left, const vec4_t &right,
                                         value_t *values, size_t max,
                                         bool *next, u_int8_t key_idx) const {
  int return_code = 0;
  if (key_idx == 0) {
    // range search
    return_code = bplus_tree_zmap::search_range(left, right, values, max, next);
  } else {
    // using the zonemap
    // scan all the leaf nodes
    if (left == NULL || keycmp(*left, right) > 0) return -1;
    size_t count = 0;
    off_t off = meta.leaf_offset;
    record_t<bpt::vec4_t> *b, *e;

    leaf_node_t<bpt::vec4_t> leaf;
    while (off != 0 && count < max) {
      map(&leaf, off);
      b = begin(leaf);
      // set the end pointer of the current leaf node
      e = leaf.children + leaf.n;
      // copy the values
      for (; b != e && count < max; ++b) {
        if (b->key.k[key_idx] >= (*left).k[key_idx] &&
            b->key.k[key_idx] < right.k[key_idx]) {
          values[count++] = b->value;
        }
      }
      // iterate to the next leaf
      off = leaf.next;
    }

    // mark for next iteration
    if (next != NULL) {
      if (count == max && off != 0) {
        // end due to the limitation of value arr size
        *next = true;
        *left = b->key;
      } else {
        // all the result is returned
        *next = false;
      }
    }
    // the size of the result
    return_code = count;
  }
  return return_code;
}

void bplus_tree_zmap::node_create(off_t offset, internal_node_zmap_t *node,
                                  internal_node_zmap_t *next) {
  // new sibling node
  next->parent = node->parent;
  next->next = node->next;
  next->prev = offset;
  node->next = alloc(next);
  // update next node's prev
  if (next->next != 0) {
    internal_node_zmap_t old_next;
    map(&old_next, next->next, SIZE_NO_CHILDREN_ZMAP);
    old_next.prev = node->next;
    unmap(&old_next, next->next, SIZE_NO_CHILDREN_ZMAP);
  }
  unmap(&meta, OFFSET_META);
}

}  // namespace bpt
