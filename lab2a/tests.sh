#!/bin/bash

rm -f lab2_add.csv
rm -f lab2_list.csv
touch lab2_add.csv
touch lab2_list.csv

# add
# yield and non-yield
# none sync
for t in 1 2 4 8 12
do
    for  i in 10 20 40 80 100 1000 10000 100000
    do

	./lab2_add --thread=$t --iterations=$i >> lab2_add.csv
	./lab2_add --thread=$t --iterations=$i --yield >> lab2_add.csv
    done
done

# add
# different locks with yield
for t in 1 2 4 8 12
do
    for  i in 10 20 40 80 100 1000 10000 100000
    do

	./lab2_add --thread=$t --iterations=$i --yield --sync=m >> lab2_add.csv
	./lab2_add --thread=$t --iterations=$i --yield --sync=s >> lab2_add.csv
	./lab2_add --thread=$t --iterations=$i --yield --sync=c >> lab2_add.csv	
    done
done

# add
# different locks w/o yield
# none yield
for t in 1 2 4 8 12
do
    for  i in 10 20 40 80 100 1000 10000 100000
    do

	./lab2_add --thread=$t --iterations=$i --sync=m >> lab2_add.csv
	./lab2_add --thread=$t --iterations=$i --sync=s >> lab2_add.csv
	./lab2_add --thread=$t --iterations=$i --sync=c >> lab2_add.csv	
    done
done

# list
# single thread
for  i in 10 100 1000 10000 20000
do
    ./lab2_list --thread=1 --iterations=$i --sync=m >> lab2_list.csv
    
done

# list
# none sync
# none yield
for t in 2 4 8 12
do
    for i in 1 10 100 1000
    do
	./lab2_list --threads=$t --iterations=$i >> lab2_list.csv
    done
done

# with yield
for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	for y in i d il dl
	do
	    ./lab2_list --threads=$t --iterations=$i --yield=$y >> lab2_list.csv
	    ./lab2_list --threads=$t --iterations=$i --yield=$y --sync=m >> lab2_list.csv
	    ./lab2_list	--threads=$t --iterations=$i --yield=$y	--sync=s >> lab2_list.csv
	done
    done
done

for y in i d il dl
do
    ./lab2_list --threads=12 --iterations=32 --yield=$y --sync=m >> lab2_list.csv
    ./lab2_list --threads=12 --iterations=32 --yield=$y --sync=s >> lab2_list.csv
done

# list-none-sync test ranges
for t in 1 2 4 8 12 16 24
do
    ./lab2_list --threads=$t --iterations=1000 --sync=m >> lab2_list.csv
    ./lab2_list --threads=$t --iterations=1000 --sync=s >> lab2_list.csv
done
