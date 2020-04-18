// Copyright [2020] <Copyright Owner>  [legal/copyright]
#include <assert.h>
#include <util/benchmark_utils.h>

#include <iostream>
void print(std::vector<uint32_t> const &input) {
  for (int i = 0; i < input.size(); i++) {
    std::cout << input.at(i) << ' ';
  }
}

int main(int argc, char *argv[]) {
  // test the function to convert key and recoard_id to 4D uint_32 vector
  vector<uint32_t> comp_key =
      date2keyarr("1993-01-03|1994-01-03|1995-01-03", 9999);
  assert(comp_key[0] == 1020451);
  assert(comp_key[1] == 1020963);
  assert(comp_key[2] == 1021475);
  assert(comp_key[3] == 9999);
}
