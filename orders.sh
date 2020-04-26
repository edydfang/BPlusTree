
echo "----------ORDER = 50, SIZE = 1G----------"
make dbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/1G.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/1G.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=50
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 1G----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1G.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1G.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean

echo "----------ORDER = 200, SIZE = 1G----------"
make dbgen ORDER=-DBP_ORDER=200 SOURCE=-DSOURCE='\"data/1G.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=200 SOURCE=-DSOURCE='\"data/1G.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=200
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 0.5G----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/0_5G.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/0_5G.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 1.4G----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1_4G.txt\"'
./dbgen
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1_4G.txt\"'
./zdbgen
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean



