#include "zbpt.h"

#include <assert.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>

#include "util/benchmark_utils.h"

#define KEY(key, rid) bpt::vec4_t(date2keyarr(key, rid))
#define SIZE 10000

// Provide some namespace shortcuts
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

typedef bpt::bplus_tree_zmap bt;
typedef bpt::vec4_t key;

int main() {
  bt db("zbpt_vec4.db", true);
  std::ifstream fin("data/li_short.txt", std::ios_base::binary);
  uint32_t i = 0;
  char line[1024] = {0};
  while (fin.getline(line, sizeof(line))) {
    // cout << line << std::endl;
    assert(db.insert(bpt::vec4_t(date2keyarr(line, i)), i) == 0);
    i++;
    if (i % 10000 == 0) {
      cout << "Finished " << i << " insertion." << std::endl;
    }
  }
  fin.clear();
  fin.close();
}