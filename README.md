# Zone Map Enhanced B+-Tree for Correlated Multi-Column Index

This implementation is based on another open-source B+ tree implementation.

https://github.com/zcbenz/BPlusTree (Copyright (c) 2012 Zhao Cheng)

## Files

- `bpt.h` and `bpt.cc` is the implementation of B+ tree. `predefined.h` defines the tree order, key/value type, key compare function and other tree settings, modify it to satify your need. Just include these tree files in your project to use the B+ tree.
- `zbpt.h` and `zbpt.cc` is the implementation of zone map enhanced B+ tree and it inherites the baseline B+ tree class.
- `dbgen.cc` and `zdbgen.cc` generate the database index file by inserting the data records one by one for baseline and zone map enhanced B+ tree.
- `benchmark.c` code for runing benchmark.
- `util/unit_test.cc` is the test code for baseline B+ tree.
- `dump_numbers.cc` can write some numbers into a database, so you can quickly test out the B+ tree.
- `cli.cc` is a command tool to manipulate an exisiting database.

## Compile
The code currently can be only compiled and run on Linux because some special system calls for memory management.
```
# to compile
make
```
## Benchmark

1. generate the lineitem table flat file using TPC-H benchamrk generation tool
2. `./data_process <input-data-file>` to get flat file with only three date columns
3. `./db_gen` or `./zdb_gen` to generate database file for the baseline and the zonemap enhanced version.
4. `./benchmark` to run benchmark


## License

The MIT License (MIT)
