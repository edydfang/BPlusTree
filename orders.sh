make clean
echo "----------ORDER = 50, SIZE = 1G----------"
echo "----------ORDER = 50, SIZE = 1G BPT----------"
make dbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./dbgen
done  
echo "----------ORDER = 50, SIZE = 1G ZBPT----------"
make zdbgen ORDER=-DBP_ORDER=50 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./zdbgen
done  
make benchmark ORDER=-DBP_ORDER=50
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 1G----------"
echo "----------ORDER = 100, SIZE = 1G BPT----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./dbgen
done  
echo "----------ORDER = 100, SIZE = 1G ZBPT----------"
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./zdbgen
done  
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean

echo "----------ORDER = 200, SIZE = 1G----------"
echo "----------ORDER = 200, SIZE = 1G BPT----------"
make dbgen ORDER=-DBP_ORDER=200 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./dbgen
done  
echo "----------ORDER = 200, SIZE = 1G ZBPT----------"
make zdbgen ORDER=-DBP_ORDER=200 SOURCE=-DSOURCE='\"data/1G.txt\"'
for((i=1;i<=5;i++))
do   
time ./zdbgen
done  
make benchmark ORDER=-DBP_ORDER=200
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 0.5G----------"
echo "----------ORDER = 100, SIZE = 0.5G BPT----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/0_5G.txt\"'
for((i=1;i<=5;i++))
do   
time ./dbgen
done  
echo "----------ORDER = 100, SIZE = 0.5G ZBPT----------"
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/0_5G.txt\"'
for((i=1;i<=5;i++))
do   
time ./zdbgen
done  
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean

echo "----------ORDER = 100, SIZE = 1.4G----------"
echo "----------ORDER = 100, SIZE = 1.4G BPT----------"
make dbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1_4G.txt\"'
for((i=1;i<=5;i++))
do   
time ./dbgen
done  
echo "----------ORDER = 100, SIZE = 1.4G ZBPT----------"
make zdbgen ORDER=-DBP_ORDER=100 SOURCE=-DSOURCE='\"data/1_4G.txt\"'
for((i=1;i<=5;i++))
do   
time ./zdbgen
done 
make benchmark ORDER=-DBP_ORDER=100
./benchmark
make clean



