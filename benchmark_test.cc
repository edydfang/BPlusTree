#include <assert.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>

#include "bpt.h"
#include "util/benchmark_utils.h"

#define KEY(key, rid) bpt::vec4_t(date2keyarr(key, rid))
#define SIZE 10000

// Provide some namespace shortcuts
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

typedef bpt::bplus_tree<bpt::vec4_t> bt;
typedef bpt::vec4_t key;

void load_db(bt& db) {
    std::ifstream fin("data/test.txt", std::ios_base::binary);
    uint32_t i = 0;
    char line[1024] = {0};
    while (fin.getline(line, sizeof(line))) {
        cout << line << std::endl;
        assert(db.insert(bpt::vec4_t(date2keyarr(line, i)), i) == 0);
        i++;
    }
    fin.clear();
    fin.close();
}

int test(bt db, u_int8_t key_idx, int range, bpt::value_t* values) {
    bool next = true;
    int return_num = 0;
    // key left_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
    switch (range) {
        case 1: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            // key r_key = KEY("1993-12-05|1994-01-08|1994-01-02", 1);
            while (next) {
                return_num +=
                    db.search_single(l_key, values, SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 2: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1993-12-06|1994-01-09|1994-01-03", 2);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 4: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1993-12-08|1994-01-11|1994-01-05", 4);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 8: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1993-12-12|1994-01-15|1994-01-09", 8);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 16: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1993-12-20|1994-01-23|1994-01-17", 16);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 30: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1994-01-04|1994-02-07|1994-02-01", 30);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
            }
            return return_num;
        }
        case 60: {
            key l_key = KEY("1993-12-04|1994-01-07|1994-01-01", 0);
            key r_key = KEY("1994-02-04|1994-03-07|1994-03-01", 60);
            while (next) {
                return_num += db.search_range_single(&l_key, r_key, values,
                                                     SIZE, &next, key_idx);
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

    bt tree_vec4("test_vec4.db", true);
    load_db(tree_vec4);
    bpt::value_t* values = new bpt::value_t[SIZE];

    int range[7] = {1, 2, 4, 8, 16, 30, 60};
    for (u_int8_t i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            // Get the starting timestamp
            start = high_resolution_clock::now();

            int num = test(tree_vec4, i, range[j], values);

            // Get the ending timestamp
            end = high_resolution_clock::now();
            // Convert the calculated duration to a double using the standard
            // library
            duration_sec =
                std::chrono::duration_cast<duration<double, std::milli> >(
                    end - start);
            // Durations are converted to milliseconds already thanks to
            // std::chrono::duration_cast
            printf("key_idx = %d, range = %d, num = %d, time = %lf ms\n", i,
                   range[j], num, duration_sec.count());
            // cout << "Total time: " << duration_sec.count() << "ms\n";
        }
    }

    return 0;
}