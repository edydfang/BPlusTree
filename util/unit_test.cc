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
#include <bpt.h>
#include <util/benchmark_utils.h>

#include <algorithm>
using bpt::bplus_tree;
using bpt::strkey_t;
using bpt::vec4_t;

int main(int argc, char *argv[]) {
  {
    bplus_tree<bpt::strkey_t> tree("test.db", true);
    assert(tree.meta.order == 4);
    assert(tree.meta.value_size == sizeof(bpt::value_t));
    assert(tree.meta.key_size == sizeof(bpt::strkey_t));
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 1);
    assert(tree.meta.height == 1);
    PRINT("EmptyTree");
  }

  {
    bplus_tree<bpt::strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.value_size == sizeof(bpt::value_t));
    assert(tree.meta.key_size == sizeof(bpt::vec4_t));
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 1);
    assert(tree.meta.height == 1);
    PRINT("ReReadEmptyTree");
    assert(tree.insert(bpt::strkey_t("t2"), 2) == 0);
    assert(tree.insert(bpt::strkey_t("t4"), 4) == 0);
    assert(tree.insert(bpt::strkey_t("t1"), 1) == 0);
    assert(tree.insert(bpt::strkey_t("t3"), 3) == 0);
  }

  {
    bplus_tree<bpt::strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.value_size == sizeof(bpt::value_t));
    assert(tree.meta.key_size == sizeof(bpt::strkey_t));
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 1);
    assert(tree.meta.height == 1);

    bpt::leaf_node_t<bpt::strkey_t> leaf;
    tree.map(&leaf, tree.search_leaf(bpt::strkey_t("t1")));
    assert(leaf.n == 4);
    assert(bpt::keycmp(leaf.children[0].key, bpt::strkey_t("t1")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, bpt::strkey_t("t2")) == 0);
    assert(bpt::keycmp(leaf.children[2].key, bpt::strkey_t("t3")) == 0);
    assert(bpt::keycmp(leaf.children[3].key, bpt::strkey_t("t4")) == 0);
    bpt::value_t value;

    assert(tree.search(bpt::strkey_t("t1"), &value) == 0);
    assert(value == 1);
    assert(tree.search(bpt::strkey_t("t2"), &value) == 0);
    assert(value == 2);
    assert(tree.search(bpt::strkey_t("t3"), &value) == 0);
    assert(value == 3);
    assert(tree.search(bpt::strkey_t("t4"), &value) == 0);
    assert(value == 4);
    assert(tree.insert(bpt::strkey_t("t1"), 4) == 1);
    assert(tree.insert(bpt::strkey_t("t2"), 4) == 1);
    assert(tree.insert(bpt::strkey_t("t3"), 4) == 1);
    assert(tree.insert(bpt::strkey_t("t4"), 4) == 1);
    assert(tree.insert(bpt::strkey_t("t5"), 5) == 0);
    PRINT("Insert4Elements");
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.value_size == sizeof(bpt::value_t));
    assert(tree.meta.key_size == sizeof(strkey_t));
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 2);
    assert(tree.meta.height == 1);

    bpt::internal_node_t<strkey_t> index;
    off_t index_off = tree.search_index(strkey_t("t1"));
    tree.map(&index, index_off);
    assert(index.n == 2);
    assert(index.parent == 0);
    assert(bpt::keycmp(index.children[0].key, strkey_t("t4")) == 0);

    bpt::leaf_node_t<strkey_t> leaf1, leaf2;
    off_t leaf1_off = tree.search_leaf(strkey_t("t1"));
    assert(leaf1_off == index.children[0].child);
    tree.map(&leaf1, leaf1_off);
    assert(leaf1.n == 3);
    assert(bpt::keycmp(leaf1.children[0].key, strkey_t("t1")) == 0);
    assert(bpt::keycmp(leaf1.children[1].key, strkey_t("t2")) == 0);
    assert(bpt::keycmp(leaf1.children[2].key, strkey_t("t3")) == 0);

    off_t leaf2_off = tree.search_leaf(strkey_t("t4"));
    assert(leaf1.next == leaf2_off);
    assert(leaf2_off == index.children[1].child);
    tree.map(&leaf2, leaf2_off);
    assert(leaf2.n == 2);
    assert(bpt::keycmp(leaf2.children[0].key, strkey_t("t4")) == 0);
    assert(bpt::keycmp(leaf2.children[1].key, strkey_t("t5")) == 0);

    PRINT("SplitLeafBy2");
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.insert(strkey_t("t1"), 4) == 1);
    assert(tree.insert(strkey_t("t2"), 4) == 1);
    assert(tree.insert(strkey_t("t3"), 4) == 1);
    assert(tree.insert(strkey_t("t4"), 4) == 1);
    assert(tree.insert(strkey_t("t5"), 4) == 1);
    bpt::value_t value;
    assert(tree.search(strkey_t("t1"), &value) == 0);
    assert(value == 1);
    assert(tree.search(strkey_t("t2"), &value) == 0);
    assert(value == 2);
    assert(tree.search(strkey_t("t3"), &value) == 0);
    assert(value == 3);
    assert(tree.search(strkey_t("t4"), &value) == 0);
    assert(value == 4);
    assert(tree.search(strkey_t("t5"), &value) == 0);
    assert(value == 5);
    PRINT("Search2Leaf");

    assert(tree.insert(strkey_t("t6"), 6) == 0);
    assert(tree.insert(strkey_t("t7"), 7) == 0);
    assert(tree.insert(strkey_t("t8"), 8) == 0);
    assert(tree.insert(strkey_t("t9"), 9) == 0);
    assert(tree.insert(strkey_t("ta"), 10) == 0);
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 3);
    assert(tree.meta.height == 1);

    bpt::internal_node_t<strkey_t> index;
    off_t index_off = tree.search_index(strkey_t("t8"));
    tree.map(&index, index_off);
    assert(index.n == 3);
    assert(index.parent == 0);
    assert(bpt::keycmp(index.children[0].key, strkey_t("t4")) == 0);
    assert(bpt::keycmp(index.children[1].key, strkey_t("t7")) == 0);

    bpt::leaf_node_t<strkey_t> leaf1, leaf2, leaf3;
    off_t leaf1_off = tree.search_leaf(strkey_t("t3"));
    off_t leaf2_off = tree.search_leaf(strkey_t("t5"));
    off_t leaf3_off = tree.search_leaf(strkey_t("ta"));
    tree.map(&leaf1, leaf1_off);
    tree.map(&leaf2, leaf2_off);
    tree.map(&leaf3, leaf3_off);
    assert(index.children[0].child == leaf1_off);
    assert(index.children[1].child == leaf2_off);
    assert(index.children[2].child == leaf3_off);
    assert(leaf1.next == leaf2_off);
    assert(leaf2.next == leaf3_off);
    assert(leaf3.next == 0);
    PRINT("SplitLeafBy3");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    assert(tree.meta.order == 4);
    assert(tree.insert(strkey_t("t00"), 0) == 0);
    assert(tree.insert(strkey_t("t01"), 1) == 0);
    assert(tree.insert(strkey_t("t02"), 2) == 0);
    assert(tree.insert(strkey_t("t03"), 3) == 0);
    assert(tree.insert(strkey_t("t04"), 4) == 0);
    assert(tree.insert(strkey_t("t05"), 5) == 0);
    assert(tree.insert(strkey_t("t06"), 6) == 0);
    assert(tree.insert(strkey_t("t07"), 7) == 0);
    assert(tree.insert(strkey_t("t08"), 8) == 0);
    assert(tree.insert(strkey_t("t09"), 9) == 0);
    assert(tree.insert(strkey_t("t10"), 10) == 0);
    assert(tree.insert(strkey_t("t11"), 11) == 0);
    assert(tree.insert(strkey_t("t12"), 12) == 0);
    assert(tree.insert(strkey_t("t13"), 13) == 0);
    assert(tree.insert(strkey_t("t14"), 14) == 0);
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 3);
    assert(tree.meta.leaf_node_num == 5);
    assert(tree.meta.height == 2);

    bpt::internal_node_t<strkey_t> node1, node2, root;
    tree.map(&root, tree.meta.root_offset);
    off_t node1_off = tree.search_index(strkey_t("t03"));
    off_t node2_off = tree.search_index(strkey_t("t14"));
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    assert(root.n == 2);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(bpt::keycmp(root.children[0].key, strkey_t("t09")) == 0);
    assert(node1.n == 3);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("t03")) == 0);
    assert(bpt::keycmp(node1.children[1].key, strkey_t("t06")) == 0);
    assert(node2.n == 2);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("t12")) == 0);

    bpt::value_t value;
    for (int i = 0; i < 10; i++) {
      char key[8] = {0};
      sprintf(key, "t0%d", i);
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    for (int i = 10; i < 14; i++) {
      char key[8] = {0};
      sprintf(key, "t%d", i);
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }

    PRINT("CreateNewRoot");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    assert(tree.meta.order == 4);
    for (int i = 0; i < 12; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 4);
    assert(tree.meta.height == 1);

    bpt::internal_node_t<strkey_t> node;
    tree.map(&node, tree.meta.root_offset);
    assert(node.n == 4);
    // assert(bpt::keycmp(node.children[0].key, strkey_t("11")) == 0);
    // assert(bpt::keycmp(node.children[1].key, strkey_t("3")) == 0);
    // assert(bpt::keycmp(node.children[2].key, strkey_t("6")) == 0);

    off_t off1 = tree.search_leaf(strkey_t("0"));
    off_t off2 = tree.search_leaf(strkey_t("11"));
    off_t off3 = tree.search_leaf(strkey_t("5"));
    off_t off4 = tree.search_leaf(strkey_t("6"));
    assert(node.children[0].child == off1);
    assert(node.children[1].child == off2);
    assert(node.children[2].child == off3);
    assert(node.children[3].child == off4);
    bpt::leaf_node_t<strkey_t> node1, node2, node3, node4;
    tree.map(&node1, off1);
    tree.map(&node2, off2);
    tree.map(&node3, off3);
    tree.map(&node4, off4);
    assert(node1.n == 3);
    assert(node2.n == 2);
    assert(node3.n == 3);
    assert(node4.n == 4);

    for (int i = 0; i < 12; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    PRINT("SplitInTheBeginning");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 10; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
    assert(tree.insert(strkey_t("51"), 51) == 0);
    assert(tree.insert(strkey_t("52"), 52) == 0);
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 4);
    assert(tree.meta.height == 1);

    bpt::internal_node_t<strkey_t> node;
    tree.map(&node, tree.meta.root_offset);
    assert(node.n == 4);
    assert(bpt::keycmp(node.children[0].key, strkey_t("3")) == 0);
    assert(bpt::keycmp(node.children[1].key, strkey_t("51")) == 0);
    assert(bpt::keycmp(node.children[2].key, strkey_t("6")) == 0);

    off_t off1 = tree.search_leaf(strkey_t("0"));
    off_t off2 = tree.search_leaf(strkey_t("3"));
    off_t off3 = tree.search_leaf(strkey_t("51"));
    off_t off4 = tree.search_leaf(strkey_t("6"));
    assert(node.children[0].child == off1);
    assert(node.children[1].child == off2);
    assert(node.children[2].child == off3);
    assert(node.children[3].child == off4);
    bpt::leaf_node_t<strkey_t> node1, node2, node3, node4;
    tree.map(&node1, off1);
    tree.map(&node2, off2);
    tree.map(&node3, off3);
    tree.map(&node4, off4);
    assert(node1.n == 3);
    assert(node2.n == 3);
    assert(node3.n == 2);
    assert(node4.n == 4);
    for (int i = 0; i < 10; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    assert(tree.insert(strkey_t("51"), 51) == 1);
    assert(tree.insert(strkey_t("52"), 52) == 1);
    PRINT("SplitInTheMiddle");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 15; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 3);
    assert(tree.meta.leaf_node_num == 5);
    assert(tree.meta.height == 2);

    bpt::internal_node_t<strkey_t> node1, node2, root;
    tree.map(&root, tree.meta.root_offset);
    off_t node1_off = tree.search_index(strkey_t("0"));
    off_t node2_off = tree.search_index(strkey_t("6"));
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    assert(root.n == 2);
    assert(bpt::keycmp(root.children[0].key, strkey_t("3")) == 0);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(node1.n == 3);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("11")) == 0);
    assert(bpt::keycmp(node1.children[1].key, strkey_t("14")) == 0);
    assert(node2.n == 2);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("6")) == 0);
    for (int i = 0; i < 15; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    PRINT("CreateNewRootInMiddle");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 30; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.internal_node_num == 5);
    assert(tree.meta.leaf_node_num == 10);
    assert(tree.meta.height == 2);

    bpt::internal_node_t<strkey_t> node1, node2, node3, node4, root;
    tree.map(&root, tree.meta.root_offset);
    off_t node1_off = tree.search_index(strkey_t("11"));
    off_t node2_off = tree.search_index(strkey_t("22"));
    off_t node3_off = tree.search_index(strkey_t("28"));
    off_t node4_off = tree.search_index(strkey_t("6"));
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    tree.map(&node3, node3_off);
    tree.map(&node4, node4_off);
    assert(root.prev == 0);
    assert(root.next == 0);
    assert(node1.prev == 0);
    assert(node1.next == node2_off);
    assert(node2.prev == node1_off);
    assert(node2.next == node3_off);
    assert(node3.prev == node2_off);
    assert(node3.next == node4_off);
    assert(node4.prev == node3_off);
    assert(node4.next == 0);
    assert(root.n == 4);
    assert(bpt::keycmp(root.children[0].key, strkey_t("17")) == 0);
    assert(bpt::keycmp(root.children[1].key, strkey_t("25")) == 0);
    assert(bpt::keycmp(root.children[2].key, strkey_t("3")) == 0);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(root.children[2].child == node3_off);
    assert(root.children[3].child == node4_off);
    assert(node1.n == 3);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("11")) == 0);
    assert(bpt::keycmp(node1.children[1].key, strkey_t("14")) == 0);
    assert(node2.n == 3);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("2")) == 0);
    assert(bpt::keycmp(node2.children[1].key, strkey_t("22")) == 0);
    assert(node3.n == 2);
    assert(bpt::keycmp(node3.children[0].key, strkey_t("28")) == 0);
    assert(node4.n == 2);
    assert(bpt::keycmp(node4.children[0].key, strkey_t("6")) == 0);

    for (int i = 0; i < 30; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    PRINT("SplitAfterNewRoot");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 49; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.internal_node_num == 9);
    assert(tree.meta.leaf_node_num == 17);
    assert(tree.meta.height == 3);

    bpt::internal_node_t<strkey_t> root;
    tree.map(&root, tree.meta.root_offset);
    assert(root.n == 2);
    assert(bpt::keycmp(root.children[0].key, strkey_t("3")) == 0);

    bpt::internal_node_t<strkey_t> node1, node2;
    tree.map(&node1, root.children[0].child);
    tree.map(&node2, root.children[1].child);
    assert(node1.n == 3);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("17")) == 0);
    assert(bpt::keycmp(node1.children[1].key, strkey_t("25")) == 0);
    assert(node2.n == 3);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("4")) == 0);
    assert(bpt::keycmp(node2.children[1].key, strkey_t("45")) == 0);

    bpt::internal_node_t<strkey_t> node3, node4, node5;
    tree.map(&node3, node2.children[0].child);
    assert(node3.n == 4);
    assert(bpt::keycmp(node3.children[0].key, strkey_t("32")) == 0);
    assert(bpt::keycmp(node3.children[1].key, strkey_t("35")) == 0);
    assert(bpt::keycmp(node3.children[2].key, strkey_t("38")) == 0);
    tree.map(&node4, node2.children[1].child);
    assert(node4.n == 2);
    assert(bpt::keycmp(node4.children[0].key, strkey_t("42")) == 0);
    tree.map(&node5, node2.children[2].child);
    assert(node5.n == 3);
    assert(bpt::keycmp(node5.children[0].key, strkey_t("48")) == 0);
    assert(bpt::keycmp(node5.children[1].key, strkey_t("6")) == 0);

    for (int i = 0; i < 49; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    PRINT("DanglingMiddleKey");
  }

  int numbers[SIZE];
  for (int i = 0; i < SIZE; i++) numbers[i] = i;

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < SIZE; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.insert(strkey_t(key), numbers[i]) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    for (int i = 0; i < SIZE; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == numbers[i]);
    }
    PRINT("InsertManyKeys");
  }

  std::reverse(numbers, numbers + SIZE);
  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < SIZE; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.insert(strkey_t(key), numbers[i]) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    for (int i = 0; i < SIZE; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == numbers[i]);
    }
    PRINT("InsertManyKeysReverse");
  }

  for (int i = 0; i < 10; i++) {
    std::random_shuffle(numbers, numbers + SIZE);
    {
      bplus_tree<strkey_t> tree("test.db", true);
      for (int i = 0; i < SIZE; i++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[i]);
        assert(tree.insert(strkey_t(key), numbers[i]) == 0);
      }
    }

    {
      bplus_tree<strkey_t> tree("test.db");
      for (int i = 0; i < SIZE; i++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[i]);
        bpt::value_t value;
        assert(tree.search(strkey_t(key), &value) == 0);
        assert(value == numbers[i]);
      }
    }
  }

  PRINT("InsertManyKeysRandom");

  {
    for (int i = 0; i < SIZE; i++) numbers[i] = i;

    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < SIZE; i++) {
      char key[8] = {0};
      sprintf(key, "%04d", numbers[i]);
      assert(tree.insert(strkey_t(key), numbers[i]) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    int start = rand() % (SIZE - 20);
    int end = rand() % (SIZE - start) + start;
    char bufkey1[8] = {0};
    char bufkey2[8] = {0};
    sprintf(bufkey1, "%04d", start);
    sprintf(bufkey2, "%04d", end);
    strkey_t key1(bufkey1), key2(bufkey2);
    bpt::value_t values[end - start + 1];
    assert(tree.search_range(&key1, key2, values, end - start + 1) ==
           end - start + 1);

    for (int i = start; i <= end; i++) {
      char key[8] = {0};
      sprintf(key, "%04d", i);
      assert(i == values[i - start]);
    }

    bool next;
    assert(tree.search_range(&key1, key2, values, end - start + 100) ==
           end - start + 1);
    assert(tree.search_range(&key1, key2, values, end - start + 100, &next) ==
           end - start + 1);
    assert(next == false);

    PRINT("SearchRangeSuccess");

    assert(tree.search_range(&key1, key1, values, end - start + 1) == 1);
    assert(tree.search_range(&key1, key1, values, end - start + 1, &next) == 1);
    assert(next == false);
    assert(tree.search_range(&key2, key2, values, end - start + 1) == 1);
    assert(tree.search_range(&key2, key2, values, end - start + 1, &next) == 1);
    assert(next == false);
    PRINT("SearchRangeSameKey");

    assert(tree.search_range(&key2, key1, values, end - start + 1) == -1);
    assert(tree.search_range(&key1, key2, values, end - start) == end - start);
    assert(tree.search_range(&key1, key2, values, end - start, &next) ==
           end - start);
    assert(next == true);

    PRINT("SearchRangeFailed");
  }

  for (int i = 0; i < 2; i++) {
    std::random_shuffle(numbers, numbers + SIZE);
    {
      bplus_tree<strkey_t> tree("test.db", true);
      for (int i = 0; i < SIZE; i++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[i]);
        assert(tree.insert(strkey_t(key), numbers[i]) == 0);
      }
    }

    {
      bplus_tree<strkey_t> tree("test.db");
      std::random_shuffle(numbers, numbers + SIZE);
      for (int i = 0; i < SIZE; i++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[i]);
        assert(tree.update(strkey_t(key), numbers[i] + 1) == 0);
      }
    }

    {
      bplus_tree<strkey_t> tree("test.db");
      for (int i = 0; i < SIZE; i++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[i]);
        bpt::value_t value;
        assert(tree.search(strkey_t(key), &value) == 0);
        assert(value == numbers[i] + 1);
      }

      for (int i = SIZE; i < SIZE * 2; i++) {
        char key[8] = {0};
        sprintf(key, "%d", i);
        bpt::value_t value;
        assert(tree.search(strkey_t(key), &value) != 0);
        assert(tree.update(strkey_t(key), i) != 0);
      }
    }
  }

  PRINT("UpdateManyKeysRandom");

  {
    bplus_tree<strkey_t> tree("test.db");
    bpt::leaf_node_t<strkey_t> leaf;
    off_t offset = tree.meta.leaf_offset;
    off_t last = 0;
    size_t counter = 0;
    while (offset != 0) {
      tree.map(&leaf, offset);
      ++counter;
      assert(last == leaf.prev);
      last = offset;
      offset = leaf.next;
    }
    assert(counter == tree.meta.leaf_node_num);

    PRINT("LeafsList");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    assert(tree.insert(strkey_t("t2"), 2) == 0);
    assert(tree.insert(strkey_t("t4"), 4) == 0);
    assert(tree.insert(strkey_t("t1"), 1) == 0);
    assert(tree.insert(strkey_t("t3"), 3) == 0);
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.remove(strkey_t("t9")) != 0);
    assert(tree.remove(strkey_t("t3")) == 0);
    assert(tree.remove(strkey_t("t3")) != 0);

    bpt::leaf_node_t<strkey_t> leaf;
    tree.map(&leaf, tree.meta.leaf_offset);
    assert(leaf.n == 3);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("t1")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("t2")) == 0);
    assert(bpt::keycmp(leaf.children[2].key, strkey_t("t4")) == 0);
    assert(tree.remove(strkey_t("t1")) == 0);
    tree.map(&leaf, tree.meta.leaf_offset);
    assert(leaf.n == 2);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("t2")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("t4")) == 0);
    assert(tree.remove(strkey_t("t2")) == 0);
    tree.map(&leaf, tree.meta.leaf_offset);
    assert(leaf.n == 1);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("t4")) == 0);
    assert(tree.remove(strkey_t("t4")) == 0);
    tree.map(&leaf, tree.meta.leaf_offset);
    assert(leaf.n == 0);
    assert(tree.remove(strkey_t("t4")) != 0);

    PRINT("RemoveInRootLeaf");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 10; i++) {
      char key[8] = {0};
      sprintf(key, "%02d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    bpt::leaf_node_t<strkey_t> leaf;
    bpt::internal_node_t<strkey_t> node;
    assert(tree.meta.leaf_node_num == 3);
    assert(tree.meta.internal_node_num == 1);

    // | 3 6  |
    // | 0 1 2 | 3 4 5 | 6 7 8 9 |
    tree.map(&node, tree.meta.root_offset);
    assert(bpt::keycmp(node.children[0].key, strkey_t("03")) == 0);
    assert(bpt::keycmp(node.children[1].key, strkey_t("06")) == 0);
    assert(tree.remove(strkey_t("03")) == 0);
    assert(tree.remove(strkey_t("04")) == 0);
    // | 2 6  |
    // | 0 1 | 2 5 | 6 7 8 9 |
    tree.map(&node, tree.meta.root_offset);
    assert(bpt::keycmp(node.children[0].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(node.children[1].key, strkey_t("06")) == 0);
    tree.map(&leaf, tree.search_leaf(strkey_t("00")));
    assert(leaf.parent == tree.meta.root_offset);
    assert(leaf.n == 2);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("00")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("01")) == 0);
    tree.map(&leaf, tree.search_leaf(strkey_t("05")));
    assert(leaf.parent == tree.meta.root_offset);
    assert(leaf.n == 2);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("05")) == 0);
    assert(tree.remove(strkey_t("05")) == 0);
    // | 2 7  |
    // | 0 1 | 2 6 | 7 8 9 |
    tree.map(&node, tree.meta.root_offset);
    assert(node.n == 3);
    assert(bpt::keycmp(node.children[0].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(node.children[1].key, strkey_t("07")) == 0);
    tree.map(&leaf, tree.search_leaf(strkey_t("04")));
    assert(leaf.parent == tree.meta.root_offset);
    assert(leaf.n == 2);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("06")) == 0);
    tree.map(&leaf, tree.search_leaf(strkey_t("07")));
    assert(leaf.parent == tree.meta.root_offset);
    assert(leaf.n == 3);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("07")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("08")) == 0);
    assert(bpt::keycmp(leaf.children[2].key, strkey_t("09")) == 0);

    bpt::value_t value;
    assert(tree.search(strkey_t("00"), &value) == 0);
    assert(value == 0);
    assert(tree.search(strkey_t("01"), &value) == 0);
    assert(value == 1);
    assert(tree.search(strkey_t("02"), &value) == 0);
    assert(value == 2);
    assert(tree.search(strkey_t("03"), &value) != 0);
    assert(tree.search(strkey_t("04"), &value) != 0);
    assert(tree.search(strkey_t("05"), &value) != 0);
    assert(tree.search(strkey_t("06"), &value) == 0);
    assert(value == 6);
    assert(tree.search(strkey_t("07"), &value) == 0);
    assert(value == 7);
    assert(tree.search(strkey_t("08"), &value) == 0);
    assert(value == 8);
    assert(tree.search(strkey_t("09"), &value) == 0);
    assert(value == 9);

    PRINT("RemoveWithBorrow");
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    bpt::internal_node_t<strkey_t> node;
    assert(tree.meta.leaf_node_num == 3);
    assert(tree.meta.internal_node_num == 1);

    // | 2 7  |
    // | 0 1 | 2 6 | 7 8 9 |
    assert(tree.remove(strkey_t("00")) == 0);
    // | 7  |
    // | 1 2 6 | 7 8 9 |
    assert(tree.meta.leaf_node_num == 2);
    assert(tree.meta.internal_node_num == 1);
    tree.map(&node, tree.meta.root_offset);
    assert(node.n == 2);
    assert(bpt::keycmp(node.children[0].key, strkey_t("07")) == 0);
    off_t leaf1_off, leaf2_off;
    leaf1_off = tree.search_leaf(strkey_t("01"));
    leaf2_off = tree.search_leaf(strkey_t("07"));
    assert(leaf1_off == node.children[0].child);
    assert(leaf2_off == node.children[1].child);
    bpt::leaf_node_t<strkey_t> leaf1, leaf2;
    tree.map(&leaf1, leaf1_off);
    tree.map(&leaf2, leaf2_off);
    assert(leaf1.n == 3);
    assert(leaf1.next == leaf2_off);
    assert(bpt::keycmp(leaf1.children[0].key, strkey_t("01")) == 0);
    assert(bpt::keycmp(leaf1.children[1].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(leaf1.children[2].key, strkey_t("06")) == 0);
    assert(leaf2.n == 3);
    assert(leaf2.next == 0);
    assert(tree.remove(strkey_t("09")) == 0);
    tree.map(&leaf2, leaf2_off);
    assert(leaf2.n == 2);
    assert(leaf2.next == 0);
    assert(bpt::keycmp(leaf2.children[0].key, strkey_t("07")) == 0);
    assert(bpt::keycmp(leaf2.children[1].key, strkey_t("08")) == 0);
    assert(tree.remove(strkey_t("01")) == 0);
    assert(tree.remove(strkey_t("08")) == 0);
    // |  |
    // | 2 6 7 |
    assert(tree.meta.leaf_node_num == 1);
    assert(tree.meta.internal_node_num == 1);
    tree.map(&node, tree.meta.root_offset);
    assert(node.n == 1);
    off_t offset;
    offset = tree.search_leaf(strkey_t("02"));
    assert(offset == node.children[0].child);
    bpt::leaf_node_t<strkey_t> leaf;
    tree.map(&leaf, offset);
    assert(leaf.n == 3);
    assert(leaf.next == 0);
    assert(leaf.prev == 0);
    assert(bpt::keycmp(leaf.children[0].key, strkey_t("02")) == 0);
    assert(bpt::keycmp(leaf.children[1].key, strkey_t("06")) == 0);
    assert(bpt::keycmp(leaf.children[2].key, strkey_t("07")) == 0);

    PRINT("RemoveWithMerge");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 15; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    // | 3  |
    // | 11 14  | 6  |
    // | 0 1 10 | 11 12 13 | 14 2 | 3 4 5 | 6 7 8 9 |
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 3);
    assert(tree.meta.leaf_node_num == 5);
    assert(tree.meta.height == 2);
    assert(tree.remove(strkey_t("6")) == 0);
    assert(tree.remove(strkey_t("7")) == 0);
    assert(tree.remove(strkey_t("8")) == 0);
    assert(tree.remove(strkey_t("9")) == 0);
    // | 14  |
    // | 11  | 3  |
    // | 0 1 10 | 11 12 13 | 14 2 | 3 4 5 |
    assert(tree.meta.internal_node_num == 3);
    assert(tree.meta.leaf_node_num == 4);
    assert(tree.meta.height == 2);

    bpt::internal_node_t<strkey_t> node1, node2, root;
    tree.map(&root, tree.meta.root_offset);
    off_t node1_off = tree.search_index(strkey_t("0"));
    off_t node2_off = tree.search_index(strkey_t("6"));
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    assert(root.n == 2);
    assert(bpt::keycmp(root.children[0].key, strkey_t("14")) == 0);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(node1.n == 2);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("11")) == 0);
    assert(node2.n == 2);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("3")) == 0);
    for (int i = 0; i < 6; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }
    for (int i = 6; i < 10; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) != 0);
    }
    for (int i = 10; i < 15; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) == 0);
      assert(value == i);
    }

    PRINT("RemoveWithBorrowInParentLeft");
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 3);
    assert(tree.meta.leaf_node_num == 4);
    assert(tree.meta.height == 2);
    assert(tree.remove(strkey_t("0")) == 0);
    assert(tree.remove(strkey_t("11")) == 0);
    bpt::internal_node_t<strkey_t> node1, node2, root;
    tree.map(&root, tree.meta.root_offset);
    off_t node1_off = tree.search_index(strkey_t("0"));
    off_t node2_off = tree.search_index(strkey_t("6"));
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    assert(root.n == 2);
    assert(bpt::keycmp(root.children[0].key, strkey_t("14")) == 0);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(node1.n == 2);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("11")) == 0);
    assert(node2.n == 2);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("3")) == 0);
    // | 14  |
    // | 11  | 3  |
    // | 1 10 | 12 13 | 14 2 | 3 4 5 |
    assert(tree.remove(strkey_t("10")) == 0);
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    // | 14  3  |
    // | 1 12 13 | 14 2 | 3 4 5 |
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 3);
    assert(tree.meta.height == 1);
    bpt::internal_node_t<strkey_t> root;
    tree.map(&root, tree.meta.root_offset);
    assert(root.n == 3);
    assert(bpt::keycmp(root.children[0].key, strkey_t("14")) == 0);
    assert(bpt::keycmp(root.children[1].key, strkey_t("3")) == 0);
    assert(tree.insert(strkey_t("1"), 0) != 0);
    assert(tree.insert(strkey_t("2"), 0) != 0);
    assert(tree.insert(strkey_t("3"), 0) != 0);
    assert(tree.insert(strkey_t("4"), 0) != 0);
    assert(tree.insert(strkey_t("5"), 0) != 0);
    assert(tree.insert(strkey_t("12"), 0) != 0);
    assert(tree.insert(strkey_t("13"), 0) != 0);
    assert(tree.insert(strkey_t("14"), 0) != 0);

    PRINT("RemoveWithHeightDecrease");
  }

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < 30; i++) {
      char key[8] = {0};
      sprintf(key, "%d", i);
      assert(tree.insert(strkey_t(key), i) == 0);
    }
  }

  {
    // | 17 25 3  |
    // | 11 14  | 2 22  | 28  | 6  |
    // | 0 1 10 | 11 12 13 | 14 15 16 | 17 18 19 | 2 20 21 | 22 23 24 | 25 26 27
    // | 28 29 | 3 4 5 | 6 7 8 9 |
    bplus_tree<strkey_t> tree("test.db");
    bpt::internal_node_t<strkey_t> node1, node2, node3, node4, root;
    off_t node1_off, node2_off, node3_off, node4_off;
    assert(tree.meta.order == 4);
    assert(tree.meta.internal_node_num == 5);
    assert(tree.meta.leaf_node_num == 10);
    assert(tree.meta.height == 2);
    assert(tree.remove(strkey_t("0")) == 0);
    assert(tree.remove(strkey_t("11")) == 0);
    assert(tree.remove(strkey_t("10")) == 0);
    assert(tree.remove(strkey_t("13")) == 0);
    assert(tree.remove(strkey_t("12")) == 0);
    // | 17 25 3  |
    // | 15  | 2 22  | 28  | 6  |
    // | 1 14 | 15 16 | 17 18 19 | 2 20 21 | 22 23 24 | 25 26 27 | 28 29 | 3 4 5
    // | 6 7 8 9 |
    assert(tree.remove(strkey_t("14")) == 0);
    // | 2 25 3  |
    // | 17  | 22  | 28  | 6  |
    // | 1 15 16 | 17 18 19 | 2 20 21 | 22 23 24 | 25 26 27 | 28 29 | 3 4 5 | 6
    // 7 8 9 |
    assert(tree.meta.internal_node_num == 5);
    assert(tree.meta.leaf_node_num == 8);
    assert(tree.meta.height == 2);

    node1_off = tree.search_index(strkey_t("11"));
    node2_off = tree.search_index(strkey_t("22"));
    node3_off = tree.search_index(strkey_t("28"));
    node4_off = tree.search_index(strkey_t("6"));
    tree.map(&root, tree.meta.root_offset);
    tree.map(&node1, node1_off);
    tree.map(&node2, node2_off);
    tree.map(&node3, node3_off);
    tree.map(&node4, node4_off);
    assert(root.prev == 0);
    assert(root.next == 0);
    assert(node1.prev == 0);
    assert(node1.next == node2_off);
    assert(node2.prev == node1_off);
    assert(node2.next == node3_off);
    assert(node3.prev == node2_off);
    assert(node3.next == node4_off);
    assert(node4.prev == node3_off);
    assert(node4.next == 0);
    assert(root.n == 4);
    assert(bpt::keycmp(root.children[0].key, strkey_t("2")) == 0);
    assert(bpt::keycmp(root.children[1].key, strkey_t("25")) == 0);
    assert(bpt::keycmp(root.children[2].key, strkey_t("3")) == 0);
    assert(root.children[0].child == node1_off);
    assert(root.children[1].child == node2_off);
    assert(root.children[2].child == node3_off);
    assert(root.children[3].child == node4_off);
    assert(node1.n == 2);
    assert(bpt::keycmp(node1.children[0].key, strkey_t("17")) == 0);
    assert(node2.n == 2);
    assert(bpt::keycmp(node2.children[0].key, strkey_t("22")) == 0);
    assert(node3.n == 2);
    assert(bpt::keycmp(node3.children[0].key, strkey_t("28")) == 0);
    assert(node4.n == 2);
    assert(bpt::keycmp(node4.children[0].key, strkey_t("6")) == 0);

    PRINT("RemoveWithBorrowInParentRight");
  }

  const int size2 = 119;
  for (int i = 0; i < size2; i++) numbers[i] = i;

  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.insert(strkey_t(key), numbers[i]) == 0);
    }
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.remove(strkey_t(key)) == 0);
      for (int j = i + 1; j < size2; j++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[j]);
        bpt::value_t value;
        assert(tree.search(strkey_t(key), &value) == 0);
      }
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    assert(tree.meta.internal_node_num == 1);
    assert(tree.meta.leaf_node_num == 1);
    assert(tree.meta.height == 1);
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) != 0);
    }
    PRINT("RemoveManyKeys");
  }

  std::reverse(numbers, numbers + size2);
  {
    bplus_tree<strkey_t> tree("test.db", true);
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.insert(strkey_t(key), numbers[i]) == 0);
    }
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      assert(tree.remove(strkey_t(key)) == 0);
      for (int j = i + 1; j < size2; j++) {
        char key[8] = {0};
        sprintf(key, "%d", numbers[j]);
        bpt::value_t value;
        assert(tree.search(strkey_t(key), &value) == 0);
      }
    }
  }

  {
    bplus_tree<strkey_t> tree("test.db");
    for (int i = 0; i < size2; i++) {
      char key[8] = {0};
      sprintf(key, "%d", numbers[i]);
      bpt::value_t value;
      assert(tree.search(strkey_t(key), &value) != 0);
    }

    PRINT("RemoveManyKeysReverse");
  }

  unlink("test.db");

  return 0;
}
