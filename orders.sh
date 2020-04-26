make dbgen ORDER=-DBP_ORDER=50
./dbgen
make zdbgen ORDER=-DBP_ORDER=50
./zdbgen
make benchmark ORDER=-DBP_ORDER=50
./benchmark

make dbgen ORDER=-DBP_ORDER=100
./dbgen
make zdbgen ORDER=-DBP_ORDER=100
make benchmark ORDER=-DBP_ORDER=100
./benchmark

make dbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/li_short_10.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/li_short_10.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=50
./benchmark

make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/li_short_10.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/li_short_10.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=100
./benchmark

make dbgen ORDER=-DBP_ORDER=200
./dbgen
make zdbgen ORDER=-DBP_ORDER=200
./zdbgen
make benchmark ORDER=-DBP_ORDER=200
./benchmark

