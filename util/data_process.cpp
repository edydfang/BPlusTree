#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "please put the input file as the first argument!");
    exit(EXIT_FAILURE);
  }

  std::ifstream fin(argv[1], std::ios::in);
  std::ofstream fout("data/li_short_10.txt", std::ios::trunc | std::ios::out);

  char line[1024] = {0};
  char tmp[33] = {0};

  while (fin.getline(line, sizeof(line))) {
    int cnt = 0, index = 0;
    while (cnt < 10) {
      if (line[index] == '|') {
        cnt++;
      }
      index++;
    }
    strncpy(tmp, line + index, 32);
    fout << tmp << '\n';
  }
  fin.clear();
  fin.close();
  fout.clear();
  fout.close();
  return 0;
}