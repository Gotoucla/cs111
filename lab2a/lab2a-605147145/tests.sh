#!/bin/bash

rm -f lab2_add.csv
rm -f lab2_list.csv
touch lab2_add.csv
touch lab2_list.csv

# add-yield test ranges 
for t in 1 2 4 8 12
do
    for i in 10 20 40 80 100 1000 10000 100000
    do
	./lab2_add --threads=$t --iterations=$i >> lab2_add.csv
	./lab2_add --yield --threads=$t --iterations=$i >> lab2_add.csv
    done
done

# add-yield mutex, cas test ranges
for t in 2 4 8 12
do
    for i in 10 100 1000 10000
    do
	./lab2_add --yield --sync=m --threads=$t --iterations=$i >> lab2_add.csv
	./lab2_add --yield --sync=c --threads=$t --iterations=$i >> lab2_add.csv
    done
done

# add-yield spin-lock test ranges
for t in 2 4 8 12
do
    for i in 10 100 1000
    do
	./lab2_add --yield --sync=s --threads=$t --iterations=$i >> lab2_add.csv
    done
done

# add-none sync test ranges
for t in 1 2 4 8 12
do
    ./lab2_add --threads=$t --iterations=10000 >> lab2_add.csv
    ./lab2_add --sync=s --threads=$t --iterations=10000 >> lab2_add.csv
    ./lab2_add --sync=m --threads=$t --iterations=10000 >> lab2_add.csv
    ./lab2_add --sync=c --threads=$t --iterations=10000 >> lab2_add.csv
done

# list-none-none single thread test ranges
for i in 10 100 1000 10000 20000
do
    ./lab2_list --threads=1 --iterations=$i >> lab2_list.csv
done

# list-none-none test ranges
for t in 2 4 8 12
do
    for i in 1 10 100 1000
    do
	./lab2_list --threads=$t --iterations=$i >> lab2_list.csv
    done
done

# list-yield-none test ranges
for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	for y in i d il dl
	do
	    ./lab2_list --threads=$t --iterations=$i --yield=$y >> lab2_list.csv
	    ./lab2_list --threads=$t --iterations=$i --yield=$y --sync=m >> lab2_list.csv
	    ./lab2_list --threads=$t --iterations=$i --yield=$y --sync=s >> lab2_list.csv
	done
    done
done

# list-yield-sync tests
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


