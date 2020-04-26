#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>

#include "bpt.h"
#include "util/benchmark_utils.h"
#include "zbpt.h"

#define KEY(key, rid) bpt::vec4_t(date2keyarr(key, rid))
#define SIZE 10000

// Provide some namespace shortcuts
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

typedef bpt::bplus_tree<bpt::vec4_t> bt;
typedef bpt::bplus_tree_zmap zbt;
typedef bpt::vec4_t key;

int range[7] = {1, 2, 4, 8, 16, 30, 60};

void n_days_after(const int date[3], int n, int new_date[3]) {
  int mon[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if ((date[0] % 4 == 0 && date[0] % 100 != 0) || date[0] % 400 == 0) {
    mon[2] = 29;
  }

  new_date[0] = date[0];
  new_date[1] = date[1];
  new_date[2] = date[2];
  for (int i = 1; i <= n; i++) {
    new_date[2] += 1;
    if (new_date[2] > mon[new_date[1]]) {
      new_date[2] = new_date[2] - mon[new_date[1]];
      new_date[1]++;
      if (new_date[1] > 12) {
        new_date[0]++;
        new_date[1] = 1;
        if ((new_date[0] % 4 == 0 && new_date[0] % 100 != 0) ||
            new_date[0] % 400 == 0) {
          mon[2] = 29;
        } else {
          mon[2] = 28;
        }
      }
    }
  }
}
/*
STARTDATE = 1992-01-01 ENDDATE = 1998-12-31
O_ORDERDATE uniformly distributed between STARTDATE and (ENDDATE - 151 days).
L_SHIPDATE = O_ORDERDATE + random value [1 .. 121].
L_COMMITDATE = O_ORDERDATE + random value [30 .. 90].
L_RECEIPTDATE = L_SHIPDATE + random value [1 .. 30].
*/
void generate_date(char left[], char right[][33]) {
  srand((unsigned)time(NULL));
  int order_date[3] = {rand() % (1997 - 1992 + 1) + 1992, rand() % 12 + 1,
                       rand() % 28 + 1};
  int ship_date[3];
  int commit_date[3];
  int recipt_date[3];
  n_days_after(order_date, rand() % 121 + 1, ship_date);
  n_days_after(order_date, rand() % (90 - 30 + 1) + 30, commit_date);
  n_days_after(ship_date, rand() % 30 + 1, recipt_date);
  snprintf(left, 33, "%04d-%02d-%02d|%04d-%02d-%02d|%04d-%02d-%02d",
           ship_date[0], ship_date[1], ship_date[2], commit_date[0],
           commit_date[1], commit_date[2], recipt_date[0], recipt_date[1],
           recipt_date[2]);

  int new_ship_date[3];
  int new_commit_date[3];
  int new_recipt_date[3];

  for (int i = 0; i < 7; i++) {
    n_days_after(ship_date, range[i], new_ship_date);
    n_days_after(commit_date, range[i], new_commit_date);
    n_days_after(recipt_date, range[i], new_recipt_date);
    snprintf(right[i], 33, "%04d-%02d-%02d|%04d-%02d-%02d|%04d-%02d-%02d",
             new_ship_date[0], new_ship_date[1], new_ship_date[2],
             new_commit_date[0], new_commit_date[1], new_commit_date[2],
             new_recipt_date[0], new_recipt_date[1], new_recipt_date[2]);
  }
}

int bpt_test(bt db, const u_int8_t key_idx, const key left, const key right,
             bpt::value_t* values) {
  // init the vars used in search_range_single()
  bool next = true;
  int return_num = 0;
  key l_key = left;
  const std::vector<uint32_t> all_zero = {0, 0, 0, 0};
  bpt::vec4_t next_key(all_zero);

  while (next) {
    return_num += db.search_range_single(&l_key, right, values, SIZE, &next_key,
                                         &next, key_idx);
  }
  return return_num;
}

int zbpt_test(zbt db, const u_int8_t key_idx, const key left, const key right,
              bpt::value_t* values) {
  // init the vars used in search_range_single()
  bool next = true;
  int return_num = 0;
  key l_key = left;

  int next_pos;
  std::queue<tuple<off_t, int>> state_queue;

  while (next) {
    return_num += db.search_range_single(&l_key, right, values, SIZE, &next,
                                         key_idx, &state_queue, &next_pos);
  }
  return return_num;
}

int main() {
  // init vars for timing
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;
  duration<double, std::milli> tmp;
  duration<double, std::milli> bpt_time[3][7];
  duration<double, std::milli> zbpt_time[3][7];

  for (int k = 0; k < 3; k++) {
    for (int j = 0; j < 7; j++) {
      bpt_time[k][j] = std::chrono::steady_clock::duration::zero();
      zbpt_time[k][j] = std::chrono::steady_clock::duration::zero();
    }
  }

  // init db
  bt tree_vec4("test_vec4.db", false);
  zbt z_tree_vec4("zbpt_vec4.db", false);
  bpt::value_t* values = new bpt::value_t[SIZE];

  // generate left and right date
  char left[33] = {0};
  char right[7][33] = {0};
  int dummy[1500000] = {0};
  // test for 10 times
  for (int i = 0; i < 10; i++) {
    generate_date(left, right);

    key l_key = KEY(left, 0);
    key r_key[7];
    for (int m = 0; m < 7; m++) {
      r_key[m] = KEY(right[m], std::numeric_limits<uint32_t>::max());
    }

    for (u_int8_t k = 0; k < 3; k++) {
      for (int j = 0; j < 7; j++) {
        cout << "iteration = " << i << " key_idx = " << (int)k
             << " left = " << left << " right = " << right[j] << std::endl;

        start = high_resolution_clock::now();
        int bpt_num = bpt_test(tree_vec4, k, l_key, r_key[j], values);
        end = high_resolution_clock::now();
        tmp = std::chrono::duration_cast<duration<double, std::milli>>(end -
                                                                       start);
        bpt_time[k][j] += tmp;

        for (int i = 0; i < 1500000; i++) dummy[i] = 1;
        // cout << " num = " << bpt_num << " time = " << tmp.count() << "ms"
        //      << std::endl;

        start = high_resolution_clock::now();
        int zbpt_num = zbpt_test(z_tree_vec4, k, l_key, r_key[j], values);
        end = high_resolution_clock::now();
        tmp = std::chrono::duration_cast<duration<double, std::milli>>(end -
                                                                       start);
        zbpt_time[k][j] += tmp;

        // cout << " z_num = " << zbpt_num << " time = " << tmp.count() << "ms"
        //      << std::endl;
        for (int i = 0; i < 1500000; i++) dummy[i] = 0;
      }
    }
  }
  cout << "------------bpt time--------------" << std::endl;
  for (u_int8_t k = 0; k < 3; k++) {
    for (int j = 0; j < 7; j++) {
      cout << "key_idx = " << (int)k << "range = " << range[j]
           << " time = " << bpt_time[k][j].count() / 10 << "ms" << std::endl;
    }
  }

  cout << "------------zbpt time--------------" << std::endl;
  for (u_int8_t k = 0; k < 3; k++) {
    for (int j = 0; j < 7; j++) {
      cout << "key_idx = " << (int)k << "range = " << range[j]
           << " time = " << zbpt_time[k][j].count() / 10 << "ms" << std::endl;
    }
  }
  return 0;
}