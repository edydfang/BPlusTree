#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRINT(a) \
  fprintf(stderr, "\033[33m%s\033[0m \033[32m%s\033[0m\n", a, "Passed")
#define INSERT(key, rid) \
  tree_vec4.insert(bpt::vec4_t(date2keyarr(key, rid)), rid)
#define SEARCH(key, rid) \
  tree_vec4.search_leaf(bpt::vec4_t(date2keyarr(key, rid)))
#define SEARCH_VAL(key, rid) \
  tree_vec4.search(bpt::vec4_t(date2keyarr(key, rid)), &value)
#define SIZE 128
#include <util/benchmark_utils.h>
#include <zbpt.h>

#include <algorithm>

using bpt::bplus_tree_zmap;
using bpt::vec4_t;

int main(int argc, char *argv[]) {
  {
    bplus_tree_zmap tree_vec4("test_zm.db", true);
    assert(tree_vec4.meta.order == 4);
    assert(tree_vec4.meta.value_size == sizeof(bpt::value_t));
    assert(tree_vec4.meta.key_size == sizeof(bpt::vec4_t));
    assert(tree_vec4.meta.internal_node_num == 1);
    assert(tree_vec4.meta.leaf_node_num == 1);
    assert(tree_vec4.meta.height == 1);
    PRINT("EmptyTree");
  }

  {
    bplus_tree_zmap tree_vec4("test_zm.db");
    assert(tree_vec4.meta.order == 4);
    assert(tree_vec4.meta.value_size == sizeof(bpt::value_t));
    assert(tree_vec4.meta.key_size == sizeof(bpt::vec4_t));
    assert(tree_vec4.meta.internal_node_num == 1);
    assert(tree_vec4.meta.leaf_node_num == 1);
    assert(tree_vec4.meta.height == 1);
    PRINT("ReReadEmptyTree");
    assert(INSERT("1993-01-03|1994-01-03|1995-01-03", 2) == 0);
    assert(INSERT("1993-01-04|1994-01-04|1995-01-04", 1) == 0);
    assert(INSERT("1993-01-01|1994-01-01|1995-01-01", 4) == 0);
    assert(INSERT("1993-01-02|1994-01-02|1995-01-02", 3) == 0);
  }

  {
    bplus_tree_zmap tree_vec4("test_zm.db");
    assert(tree_vec4.meta.order == 4);
    assert(tree_vec4.meta.value_size == sizeof(bpt::value_t));
    assert(tree_vec4.meta.key_size == sizeof(bpt::vec4_t));
    assert(tree_vec4.meta.internal_node_num == 1);
    assert(tree_vec4.meta.leaf_node_num == 1);
    assert(tree_vec4.meta.height == 1);

    bpt::leaf_node_t<bpt::vec4_t> leaf;
    tree_vec4.map(&leaf, SEARCH("1993-01-01|1994-01-01|1995-01-01", 4));
    assert(leaf.n == 4);
    assert(bpt::keycmp(leaf.children[3].key, leaf.children[2].key) > 0);
    assert(bpt::keycmp(leaf.children[2].key, leaf.children[1].key) > 0);
    assert(bpt::keycmp(leaf.children[1].key, leaf.children[0].key) > 0);
    bpt::value_t value;
    assert(SEARCH_VAL("1993-01-01|1994-01-01|1995-01-01", 4) == 0);
    assert(value == 4);
    assert(SEARCH_VAL("1993-01-02|1994-01-02|1995-01-02", 3) == 0);
    assert(value == 3);
    assert(SEARCH_VAL("1993-01-03|1994-01-03|1995-01-03", 2) == 0);
    assert(value == 2);
    assert(SEARCH_VAL("1993-01-04|1994-01-04|1995-01-04", 1) == 0);
    assert(value == 1);
    assert(INSERT("1993-01-01|1994-01-01|1995-01-01", 4) == 1);
    assert(INSERT("1993-01-02|1994-01-02|1995-01-02", 3) == 1);
    assert(INSERT("1993-01-03|1994-01-03|1995-01-03", 2) == 1);
    assert(INSERT("1993-01-04|1994-01-04|1995-01-04", 1) == 1);
    PRINT("Insert4Elements");
  }

  {
    bplus_tree_zmap tree_vec4("test_zm.db");
    bpt::value_t values[SIZE];
    int rid = 5;
    // insert
    for (; rid < 10; rid++) {
      char date_str[33];
      snprintf(date_str, sizeof(date_str), "1993-01-02|1994-01-%02d|1995-01-02",
               rid);
      assert(INSERT(date_str, rid) == 0);
    }
    for (; rid < 15; rid++) {
      char date_str[33];
      snprintf(date_str, sizeof(date_str), "1993-01-03|1994-01-%02d|1995-01-02",
               rid);
      assert(INSERT(date_str, rid) == 0);
    }
    // only the first column should matter
    bpt::vec4_t single_key(
        date2keyarr("1993-01-02|1994-01-10|1995-01-02", 100));
    int return_num = tree_vec4.search_single(single_key, values, SIZE);
    assert(return_num == 6);
    PRINT("SearchFirstColumnIndex");
  }

  unlink("test_zm.db");

  return 0;
}
