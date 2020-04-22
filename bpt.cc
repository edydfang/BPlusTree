#include "bpt.h"

#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>
using std::binary_search;
using std::lower_bound;
using std::swap;
using std::upper_bound;

namespace bpt {

/* helper searching function */
template <typename KEY_TYPE>
inline index_t<KEY_TYPE> *find(internal_node_t<KEY_TYPE> &node,
                               const KEY_TYPE &key) {
  if (key) {
    return upper_bound(begin(node), end(node) - 1, key);
  }
  // because the end of the index range is an empty string, so if we search the
  // empty key(when merge internal nodes), we need to return the second last one
  if (node.n > 1) {
    return node.children + node.n - 2;
  }
  return begin(node);
}
template <typename KEY_TYPE>
inline record_t<KEY_TYPE> *find(leaf_node_t<KEY_TYPE> &node,
                                const KEY_TYPE &key) {
  // lower_bound
  // Returns an iterator pointing to the first element in the
  // range [first,last) which does not compare less than val.
  return lower_bound(begin(node), end(node), key);
}

template <typename KEY_TYPE>
bplus_tree<KEY_TYPE>::bplus_tree(const char *p, bool force_empty)
    : fp(NULL), fp_level(0) {
  bzero(path, sizeof(path));
  snprintf(path, sizeof(path), "%s", p);
  if (!force_empty)
    // read tree from file; if no meta data, then view it as empty
    if (map(&meta, OFFSET_META) != 0) force_empty = true;

  if (force_empty) {
    open_file("w+");  // truncate file

    // create empty tree if file doesn't exist
    init_from_empty();
    close_file();
  }
}

template <typename KEY_TYPE>
int bplus_tree<KEY_TYPE>::search(const KEY_TYPE &key, value_t *value) const {
  leaf_node_t<KEY_TYPE> leaf;
  map(&leaf, search_leaf(key));

  // finding the record
  record_t<KEY_TYPE> *record = find(leaf, key);
  if (record != leaf.children + leaf.n) {
    // always return the lower bound
    *value = record->value;

    return keycmp(record->key, key);
  } else {
    return -1;
  }
}
template <typename KEY_TYPE>
int bplus_tree<KEY_TYPE>::search_range(KEY_TYPE *left, const KEY_TYPE &right,
                                       value_t *values, size_t max,
                                       bool *next) const {
  if (left == NULL || keycmp(*left, right) > 0) return -1;

  off_t off_left = search_leaf(*left);
  off_t off_right = search_leaf(right);
  off_t off = off_left;
  size_t i = 0;
  record_t<KEY_TYPE> *b, *e;
  //printf("off_left = %lld, off_right = %lld\n", off_left, off_right);

  leaf_node_t<KEY_TYPE> leaf;
  while (off != off_right && off != 0 && i < max) {
    map(&leaf, off);

    // start point
    if (off_left == off)
      b = find(leaf, *left);
    else
      b = begin(leaf);

    // set the end pointer of the current leaf node
    e = leaf.children + leaf.n;
    // copy the values
    for (; b != e && i < max; ++b, ++i) values[i] = b->value;
    // iterate to the next leaf
    off = leaf.next;

    // if b and i reach boundary condition simultaneously, we should
    // check whether need to change b to next leaf's begin record
    if(b == e && i == max && off != 0){
      map(&leaf, off);
      b = begin(leaf);
    }
  }
  //printf("i = %zu, b = %d, e = %d, off = %lld, off_right = %lld\n", i, b->value, e->value, off, off_right);
  // iterate the last leaf
  if (i < max) {
    map(&leaf, off_right);

    b = find(leaf, *left);
    e = upper_bound(begin(leaf), end(leaf), right);
    for (; b != e && i < max; ++b, ++i) values[i] = b->value;
  }

  //printf("i = %zu, b = %d, e = %d\n", i, b->value, e->value);
  // mark for next iteration
  if (next != NULL) {
    if (i == max && b != e) {
      // end due to the limitaion of value arr size
      *next = true;
      *left = b->key;
    } else {
      // all the result is returned
      *next = false;
    }
  }
  // the size of the result
  return i;
}
template <typename KEY_TYPE>
int bplus_tree<KEY_TYPE>::remove(const KEY_TYPE &key) {
  internal_node_t<KEY_TYPE> parent;
  leaf_node_t<KEY_TYPE> leaf;

  // find parent node
  off_t parent_off = search_index(key);
  map(&parent, parent_off);

  // find current node
  index_t<KEY_TYPE> *where = find(parent, key);
  off_t offset = where->child;
  map(&leaf, offset);

  // verify
  if (!binary_search(begin(leaf), end(leaf), key)) return -1;

  size_t min_n = meta.leaf_node_num == 1 ? 0 : meta.order / 2;
  assert(leaf.n >= min_n && leaf.n <= meta.order);

  // delete the key
  record_t<KEY_TYPE> *to_delete = find(leaf, key);
  std::copy(to_delete + 1, end(leaf), to_delete);
  leaf.n--;

  // merge or borrow
  if (leaf.n < min_n) {
    // first borrow from left
    bool borrowed = false;
    if (leaf.prev != 0) borrowed = borrow_key(false, leaf);

    // then borrow from right
    if (!borrowed && leaf.next != 0) borrowed = borrow_key(true, leaf);

    // finally we merge
    if (!borrowed) {
      assert(leaf.next != 0 || leaf.prev != 0);

      KEY_TYPE index_key;

      if (where == end(parent) - 1) {
        // if leaf is last element then merge | prev | leaf |
        assert(leaf.prev != 0);
        leaf_node_t<KEY_TYPE> prev;
        map(&prev, leaf.prev);
        index_key = begin(prev)->key;

        merge_leafs(&prev, &leaf);
        node_remove(&prev, &leaf);
        unmap(&prev, leaf.prev);
      } else {
        // else merge | leaf | next |
        assert(leaf.next != 0);
        leaf_node_t<KEY_TYPE> next;
        map(&next, leaf.next);
        index_key = begin(leaf)->key;

        merge_leafs(&leaf, &next);
        node_remove(&leaf, &next);
        unmap(&leaf, offset);
      }

      // remove parent's key
      remove_from_index(parent_off, parent, index_key);
    } else {
      unmap(&leaf, offset);
    }
  } else {
    unmap(&leaf, offset);
  }

  return 0;
}

template <typename KEY_TYPE>
int bplus_tree<KEY_TYPE>::insert(const KEY_TYPE &key, value_t value) {
  off_t parent = search_index(key);
  off_t offset = search_leaf(parent, key);
  leaf_node_t<KEY_TYPE> leaf;
  map(&leaf, offset);

  // check if we have the same key
  if (binary_search(begin(leaf), end(leaf), key)) return 1;

  if (leaf.n == meta.order) {
    // split when full

    // new sibling leaf
    leaf_node_t<KEY_TYPE> new_leaf;
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

// TODO adjust zone map
template <typename KEY_TYPE>
int bplus_tree<KEY_TYPE>::update(const KEY_TYPE &key, value_t value) {
  off_t offset = search_leaf(key);
  leaf_node_t<KEY_TYPE> leaf;
  map(&leaf, offset);

  record_t<KEY_TYPE> *record = find(leaf, key);
  if (record != leaf.children + leaf.n) {
    if (keycmp(key, record->key) == 0) {
      record->value = value;
      unmap(&leaf, offset);

      return 0;
    } else {
      return 1;
    }
  } else {
    return -1;
  }
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::remove_from_index(off_t offset,
                                             internal_node_t<KEY_TYPE> &node,
                                             const KEY_TYPE &key) {
  size_t min_n = meta.root_offset == offset ? 1 : meta.order / 2;
  assert(node.n >= min_n && node.n <= meta.order);

  // remove key
  KEY_TYPE index_key = begin(node)->key;
  index_t<KEY_TYPE> *to_delete = find(node, key);
  if (to_delete != end(node)) {
    (to_delete + 1)->child = to_delete->child;
    std::copy(to_delete + 1, end(node), to_delete);
  }
  node.n--;

  // remove to only one key
  if (node.n == 1 && meta.root_offset == offset &&
      meta.internal_node_num != 1) {
    unalloc(&node, meta.root_offset);
    meta.height--;
    meta.root_offset = node.children[0].child;
    unmap(&meta, OFFSET_META);
    return;
  }

  // merge or borrow
  if (node.n < min_n) {
    internal_node_t<KEY_TYPE> parent;
    map(&parent, node.parent);

    // first borrow from left
    bool borrowed = false;
    if (offset != begin(parent)->child)
      borrowed = borrow_key(false, node, offset);

    // then borrow from right
    if (!borrowed && offset != (end(parent) - 1)->child)
      borrowed = borrow_key(true, node, offset);

    // finally we merge
    if (!borrowed) {
      assert(node.next != 0 || node.prev != 0);

      if (offset == (end(parent) - 1)->child) {
        // if leaf is last element then merge | prev | leaf |
        assert(node.prev != 0);
        internal_node_t<KEY_TYPE> prev;
        map(&prev, node.prev);

        // merge
        index_t<KEY_TYPE> *where = find(parent, begin(prev)->key);
        reset_index_children_parent(begin(node), end(node), node.prev);
        merge_keys(where, prev, node, true);
        unmap(&prev, node.prev);
      } else {
        // else merge | leaf | next |
        assert(node.next != 0);
        internal_node_t<KEY_TYPE> next;
        map(&next, node.next);

        // merge
        index_t<KEY_TYPE> *where = find(parent, index_key);
        reset_index_children_parent(begin(next), end(next), offset);
        merge_keys(where, node, next);
        unmap(&node, offset);
      }

      // remove parent's key
      remove_from_index(node.parent, parent, index_key);
    } else {
      unmap(&node, offset);
    }
  } else {
    unmap(&node, offset);
  }
}
template <typename KEY_TYPE>
bool bplus_tree<KEY_TYPE>::borrow_key(bool from_right,
                                      internal_node_t<KEY_TYPE> &borrower,
                                      off_t offset) {
  typedef typename internal_node_t<KEY_TYPE>::child_t child_t;

  off_t lender_off = from_right ? borrower.next : borrower.prev;
  internal_node_t<KEY_TYPE> lender;
  map(&lender, lender_off);

  assert(lender.n >= meta.order / 2);
  if (lender.n != meta.order / 2) {
    child_t where_to_lend, where_to_put;

    internal_node_t<KEY_TYPE> parent;

    // swap keys, draw on paper to see why
    if (from_right) {
      where_to_lend = begin(lender);
      where_to_put = end(borrower);

      map(&parent, borrower.parent);
      child_t where =
          lower_bound(begin(parent), end(parent) - 1, (end(borrower) - 1)->key);
      where->key = where_to_lend->key;
      unmap(&parent, borrower.parent);
    } else {
      where_to_lend = end(lender) - 1;
      where_to_put = begin(borrower);

      map(&parent, lender.parent);
      child_t where = find(parent, begin(lender)->key);
      // where_to_put->key = where->key;  // We shouldn't change
      // where_to_put->key, because it just records the largest info but we only
      // changes a new one which have been the smallest one
      where->key = (where_to_lend - 1)->key;
      unmap(&parent, lender.parent);
    }

    // store
    std::copy_backward(where_to_put, end(borrower), end(borrower) + 1);
    *where_to_put = *where_to_lend;
    borrower.n++;

    // erase
    reset_index_children_parent(where_to_lend, where_to_lend + 1, offset);
    std::copy(where_to_lend + 1, end(lender), where_to_lend);
    lender.n--;
    unmap(&lender, lender_off);
    return true;
  }

  return false;
}
template <typename KEY_TYPE>
bool bplus_tree<KEY_TYPE>::borrow_key(bool from_right,
                                      leaf_node_t<KEY_TYPE> &borrower) {
  off_t lender_off = from_right ? borrower.next : borrower.prev;
  leaf_node_t<KEY_TYPE> lender;
  map(&lender, lender_off);

  assert(lender.n >= meta.order / 2);
  if (lender.n != meta.order / 2) {
    typename leaf_node_t<KEY_TYPE>::child_t where_to_lend, where_to_put;

    // decide offset and update parent's index key
    if (from_right) {
      where_to_lend = begin(lender);
      where_to_put = end(borrower);
      change_parent_child(borrower.parent, begin(borrower)->key,
                          lender.children[1].key);
    } else {
      where_to_lend = end(lender) - 1;
      where_to_put = begin(borrower);
      change_parent_child(lender.parent, begin(lender)->key,
                          where_to_lend->key);
    }

    // store
    std::copy_backward(where_to_put, end(borrower), end(borrower) + 1);
    *where_to_put = *where_to_lend;
    borrower.n++;

    // erase
    std::copy(where_to_lend + 1, end(lender), where_to_lend);
    lender.n--;
    unmap(&lender, lender_off);
    return true;
  }

  return false;
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::change_parent_child(off_t parent, const KEY_TYPE &o,
                                               const KEY_TYPE &n) {
  internal_node_t<KEY_TYPE> node;
  map(&node, parent);

  index_t<KEY_TYPE> *w = find(node, o);
  assert(w != node.children + node.n);

  w->key = n;
  unmap(&node, parent);
  if (w == node.children + node.n - 1) {
    change_parent_child(node.parent, o, n);
  }
}

template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::merge_leafs(leaf_node_t<KEY_TYPE> *left,
                                       leaf_node_t<KEY_TYPE> *right) {
  std::copy(begin(*right), end(*right), end(*left));
  left->n += right->n;
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::merge_keys(index_t<KEY_TYPE> *where,
                                      internal_node_t<KEY_TYPE> &node,
                                      internal_node_t<KEY_TYPE> &next,
                                      bool change_where_key) {
  // (end(node) - 1)->key = where->key;
  if (change_where_key) {
    where->key = (end(next) - 1)->key;
  }
  std::copy(begin(next), end(next), end(node));
  node.n += next.n;
  node_remove(&node, &next);
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::insert_record_no_split(leaf_node_t<KEY_TYPE> *leaf,
                                                  const KEY_TYPE &key,
                                                  const value_t &value) {
  record_t<KEY_TYPE> *where = upper_bound(begin(*leaf), end(*leaf), key);
  std::copy_backward(where, end(*leaf), end(*leaf) + 1);

  where->key = key;
  where->value = value;
  leaf->n++;
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::insert_key_to_index(off_t offset,
                                               const KEY_TYPE &key, off_t old,
                                               off_t after) {
  if (offset == 0) {
    // create new root node
    internal_node_t<KEY_TYPE> root;
    root.next = root.prev = root.parent = 0;
    meta.root_offset = alloc(&root);
    meta.height++;

    // insert `old` and `after`
    root.n = 2;
    root.children[0].key = key;
    root.children[0].child = old;
    root.children[1].child = after;

    unmap(&meta, OFFSET_META);
    unmap(&root, meta.root_offset);

    // update children's parent
    reset_index_children_parent(begin(root), end(root), meta.root_offset);
    return;
  }

  internal_node_t<KEY_TYPE> node;
  map(&node, offset);
  assert(node.n <= meta.order);

  if (node.n == meta.order) {
    // split when full

    internal_node_t<KEY_TYPE> new_node;
    node_create(offset, &node, &new_node);

    // find even split point
    size_t point = (node.n - 1) / 2;
    bool place_right = keycmp(key, node.children[point].key) > 0;
    if (place_right) ++point;

    // prevent the `key` being the right `middle_key`
    // example: insert 48 into |42|45| 6|  |
    if (place_right && keycmp(key, node.children[point].key) < 0) point--;

    KEY_TYPE middle_key = node.children[point].key;

    // split
    std::copy(begin(node) + point + 1, end(node), begin(new_node));
    new_node.n = node.n - point - 1;
    node.n = point + 1;

    // put the new key
    if (place_right)
      insert_key_to_index_no_split(new_node, key, after);
    else
      insert_key_to_index_no_split(node, key, after);

    unmap(&node, offset);
    unmap(&new_node, node.next);

    // update children's parent
    reset_index_children_parent(begin(new_node), end(new_node), node.next);

    // give the middle key to the parent
    // note: middle key's child is reserved
    insert_key_to_index(node.parent, middle_key, offset, node.next);
  } else {
    insert_key_to_index_no_split(node, key, after);
    unmap(&node, offset);
  }
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::insert_key_to_index_no_split(
    internal_node_t<KEY_TYPE> &node, const KEY_TYPE &key, off_t value) {
  index_t<KEY_TYPE> *where = upper_bound(begin(node), end(node) - 1, key);

  // move later index forward
  std::copy_backward(where, end(node), end(node) + 1);

  // insert this key
  where->key = key;
  where->child = (where + 1)->child;
  (where + 1)->child = value;

  node.n++;
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::reset_index_children_parent(index_t<KEY_TYPE> *begin,
                                                       index_t<KEY_TYPE> *end,
                                                       off_t parent) {
  // this function can change both internal_node_t and leaf_node_t's parent
  // field, but we should ensure that:
  // 1. sizeof(internal_node_t) <= sizeof(leaf_node_t)
  // 2. parent field is placed in the beginning and have same size
  internal_node_t<KEY_TYPE> node;
  while (begin != end) {
    map(&node, begin->child);
    node.parent = parent;
    unmap(&node, begin->child, SIZE_NO_CHILDREN);
    ++begin;
  }
}
template <typename KEY_TYPE>
off_t bplus_tree<KEY_TYPE>::search_index(const KEY_TYPE &key) const {
  off_t org = meta.root_offset;
  int height = meta.height;
  while (height > 1) {
    internal_node_t<KEY_TYPE> node;
    map(&node, org);
    // find the index that is strictly greater than the key
    // why end(node) - 1? we don't care the key of last index, if key is
    // greater than all prior indexes, the last index [end(node)-1] is
    // returned
    index_t<KEY_TYPE> *i = upper_bound(begin(node), end(node) - 1, key);
    org = i->child;
    --height;
  }

  return org;
}
template <typename KEY_TYPE>
off_t bplus_tree<KEY_TYPE>::search_leaf(off_t index,
                                        const KEY_TYPE &key) const {
  internal_node_t<KEY_TYPE> node;
  map(&node, index);

  index_t<KEY_TYPE> *i = upper_bound(begin(node), end(node) - 1, key);
  return i->child;
}

template <typename KEY_TYPE>
template <class T>
void bplus_tree<KEY_TYPE>::node_create(off_t offset, T *node, T *next) {
  // new sibling node
  next->parent = node->parent;
  next->next = node->next;
  next->prev = offset;
  node->next = alloc(next);
  // update next node's prev
  if (next->next != 0) {
    T old_next;
    map(&old_next, next->next, SIZE_NO_CHILDREN);
    old_next.prev = node->next;
    unmap(&old_next, next->next, SIZE_NO_CHILDREN);
  }
  unmap(&meta, OFFSET_META);
}

template <typename KEY_TYPE>
template <class T>
void bplus_tree<KEY_TYPE>::node_remove(T *prev, T *node) {
  unalloc(node, prev->next);
  prev->next = node->next;
  if (node->next != 0) {
    T next;
    map(&next, node->next, SIZE_NO_CHILDREN);
    next.prev = node->prev;
    unmap(&next, node->next, SIZE_NO_CHILDREN);
  }
  unmap(&meta, OFFSET_META);
}
template <typename KEY_TYPE>
void bplus_tree<KEY_TYPE>::init_from_empty() {
  // init default meta
  bzero(&meta, sizeof(meta_t));
  meta.order = BP_ORDER;
  meta.value_size = sizeof(value_t);
  meta.key_size = sizeof(KEY_TYPE);
  // TODO(edydfang): get num of key by template
  meta.num_key = KEY_TYPE::num_key;
  meta.height = 1;
  meta.slot = OFFSET_BLOCK;

  // init root node
  internal_node_t<KEY_TYPE> root;
  root.next = root.prev = root.parent = 0;
  meta.root_offset = alloc(&root);

  // init empty leaf
  leaf_node_t<KEY_TYPE> leaf;
  leaf.next = leaf.prev = 0;
  leaf.parent = meta.root_offset;
  meta.leaf_offset = root.children[0].child = alloc(&leaf);

  // save
  unmap(&meta, OFFSET_META);
  unmap(&root, meta.root_offset);
  unmap(&leaf, root.children[0].child);
}

template <>
int bpt::bplus_tree<bpt::vec4_t>::search_range_single(
    vec4_t *left, const vec4_t &right, value_t *values, size_t max,
    vec4_t *next_key, bool *next, u_int8_t key_idx) const {
  int return_code = 0;
  if (key_idx == 0) {
    // range search
    return_code =
        bplus_tree<bpt::vec4_t>::search_range(left, right, values, max, next);
  } else {
    // scan all the leaf nodes
    if (left == NULL || keycmp(*left, right) > 0) {
      *next = false;
      return -1;
    }

    // printf("key1 = %d, key2 = %d, key3 = %d, key4 = %d\n", left->k[0],
    //        left->k[1], left->k[2], left->k[3]);
    size_t count = 0;

    record_t<bpt::vec4_t> *b, *e;
    leaf_node_t<bpt::vec4_t> leaf;

    //set leaf and b to right location
    off_t off;
    if (!next_key){
      off = meta.leaf_offset;
      map(&leaf, off);
      b = begin(leaf);
    } else {
      off = search_leaf(*next_key);
      map(&leaf, off);
      b = find(leaf, *next_key);
    }
    //printf("b = %d\n", b->value);
    while (off != 0 && count < max) {
      // set the end pointer of the current leaf node
      e = leaf.children + leaf.n;
      // copy the values
      for (; b != e && count < max; ++b) {
        //printf("b.k = %d, left = %d, right = %d\n", b->key.k[key_idx], (*left).k[key_idx], right.k[key_idx]);
        if (b->key.k[key_idx] >= (*left).k[key_idx] &&
            b->key.k[key_idx] < right.k[key_idx]) {
          values[count++] = b->value;
          //printf("count = %d, val = %d\n", count, b->value);
        }
      }
      if (count >= max && b != e){
        break;
      }
      // iterate to the next leaf
      off = leaf.next;
      if (off != 0){
        map(&leaf, off);
        b = begin(leaf);
      }
    }

    //printf("next = %d, count = %d\n", *next, count);
    // mark for next iteration
    if (next != NULL) {
      if (count >= max && off != 0) {
        // end due to the limitation of value arr size
        *next = true;
        *next_key = b->key;
        //printf("next_key1 = %d, key2 = %d, key3 = %d, key4 = %d\n", next_key->k[0],
           //next_key->k[1], next_key->k[2], next_key->k[3]);
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

// search for a singe value for some single column
template <>
int bpt::bplus_tree<bpt::vec4_t>::search_single(vec4_t &key, value_t *values,
                                                size_t max, bool *next,
                                                u_int8_t key_idx) const {
  if (key_idx > 3) {
    return -1;
  }
  vec4_t left_most;
  vec4_t right_most;
  left_most.k[key_idx] = key.k[key_idx];
  right_most.k[key_idx] = key.k[key_idx];
  // if it is the first column
  // we have to set all the reamining column to max for right_most
  if (key_idx == 0) {
    right_most.k[1] = right_most.k[2] = right_most.k[3] =
        std::numeric_limits<uint32_t>::max();
  }

  int return_code = bplus_tree<bpt::vec4_t>::search_range_single(
      &left_most, right_most, values, max, NULL,next, key_idx);
  key = left_most;
  return return_code;
}

// Explicitly instantiate the template, and its member definitions
template class bplus_tree<bpt::vec4_t>;
template class bplus_tree<bpt::strkey_t>;
}  // namespace bpt
