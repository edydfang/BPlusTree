#include <assert.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>

#include "bpt.h"
#include "util/benchmark_utils.h"

#ifndef SOURCE
#define F_SOURCE "data/li_short.txt"
#else
#define F_SOURCE "data/li_short_10.txt"
#endif

#define KEY(key, rid) bpt::vec4_t(date2keyarr(key, rid))
#define SIZE 10000

// Provide some namespace shortcuts
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

typedef bpt::bplus_tree<bpt::vec4_t> bt;
typedef bpt::vec4_t key;

void dbgen() {
  bt db("test_vec4.db", true);
  std::ifstream fin(F_SOURCE, std::ios_base::binary);
  uint32_t i = 0;
  char line[1024] = {0};
  while (fin.getline(line, sizeof(line))) {
    // cout << line << std::endl;
    assert(db.insert(bpt::vec4_t(date2keyarr(line, i)), i) == 0);
    i++;
    // if (i % 10000 == 0) {
    //   cout << "Finished " << i << " insertion." << std::endl;
    // }
  }
  fin.clear();
  fin.close();
}

int main() {
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;
  duration<double, std::milli> duration_sec =
      std::chrono::steady_clock::duration::zero();
  duration<double, std::milli> tmp;

  for (int i = 0; i < 5; i++) {
    start = high_resolution_clock::now();
    dbgen();
    end = high_resolution_clock::now();
    tmp = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    duration_sec += tmp;

    cout << "Iteration = " << i << " time = " << tmp.count() << "ms"
         << std::endl;
  }

  cout << "ORDER = " << BP_ORDER << " time = " << duration_sec.count() / 5
       << "ms" << std::endl;
  return 0;
}