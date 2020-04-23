#include <assert.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>

#include "util/benchmark_utils.h"
#include "zbpt.h"

#define KEY(key, rid) bpt::vec4_t(date2keyarr(key, rid))
#define SIZE 10000

// Provide some namespace shortcuts
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

typedef bpt::bplus_tree_zmap bt;
typedef bpt::vec4_t key;

int test(bt db, u_int8_t key_idx, int range, bpt::value_t* values) {
  bool next = true;
  int return_num = 0;
  const std::vector<uint32_t> all_zero = {0, 0, 0, 0};
  bpt::vec4_t next_key(all_zero);
  int next_pos;
  // key left_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
  std::queue<tuple<off_t, int>> state_queue;

  switch (range) {
    case 1: {
      //   key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      //   // key r_key = KEY("1993-12-05|1994-01-08|1994-01-02", 1);
      //   while (next) {
      //     return_num += db.search_single(l_key, values, SIZE, &next,
      //     key_idx);
      //   }
      //   return return_num;
      key l_key = KEY("1994-10-16|1994-09-25|1994-10-19", 0);
      key r_key = KEY("1994-10-17|1994-09-26|1994-10-20", 100000000);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 2: {
      key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      key r_key = KEY("1993-12-06|1994-01-09|1994-01-03", 2);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 4: {
      key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      key r_key = KEY("1993-12-08|1994-01-11|1994-01-05", 4);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 8: {
      key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      key r_key = KEY("1993-12-12|1994-01-15|1994-01-09", 8);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 16: {
      key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      key r_key = KEY("1993-12-20|1994-01-23|1994-01-17", 16);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 30: {
      key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      key r_key = KEY("1994-01-04|1994-02-07|1994-02-01", 30);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    case 60: {
      //   key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
      //   key r_key = KEY("1994-02-04|1994-03-07|1994-03-01", 60);
      key l_key = KEY("1994-10-16|1994-09-25|1994-10-19", 0);
      key r_key = KEY("1994-12-16|1994-11-25|1994-12-19", 60);

      while (next) {
        return_num += db.search_range_single(&l_key, r_key, values, SIZE, &next,
                                             key_idx, &state_queue, &next_pos);
      }
      return return_num;
    }
    default: {
      printf("Wrong Parameter!\n");
      return 0;
    }
  }
}

int main(int argc, char** argv) {
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;
  duration<double, std::milli> duration_sec;

  cout << "Begin load!\n";
  bt tree_vec4("zbpt_vec4.db", false);
  bpt::value_t* values = new bpt::value_t[SIZE];

  cout << "Begin test!\n";
  int range[7] = {1, 2, 4, 8, 16, 30, 60};
  for (u_int8_t i = 0; i < 3; i++) {
    for (int j = 0; j < 7; j++) {
      // Get the starting timestamp
      start = high_resolution_clock::now();

      int num = test(tree_vec4, i, range[j], values);

      // Get the ending timestamp
      end = high_resolution_clock::now();
      // Convert the calculated duration to a double using the standard
      // library
      duration_sec =
          std::chrono::duration_cast<duration<double, std::milli>>(end - start);
      // Durations are converted to milliseconds already thanks to
      // std::chrono::duration_cast
      cout << "key_idx = " << (int)i << " range = " << range[j]
           << " num = " << num << " time = " << duration_sec.count() << "ms"
           << std::endl;
      // cout << "Total time: " << duration_sec.count() << "ms\n";
    }
  }

  return 0;
}