#!/bin/bash

rm -f lab2b_list.csv
touch lab2b_list.csv

for t in 1 2 4 8 12 16 24
do
    ./lab2_list --threads=$t --iterations=1000 --sync=m >> lab2b_list.csv
    ./lab2_list --threads=$t --iterations=1000 --sync=s >> lab2b_list.csv
done

for t in 1 4 8 12 16 
do
    for i in 1 2 4 8 16
    do  
	./lab2_list --threads=$t --iterations=$i --yield=id  >> lab2b_list.csv
    done 
done


for t in 1 4 8 12 16 
do
    for i in 10  20 40 80
    do  
	./lab2_list --threads=$t --iterations=$i --yield=id  --sync=s >> lab2b_list.csv
	./lab2_list --threads=$t --iterations=$i --yield=id  --sync=m >> lab2b_list.csv
    done
done


for t in 1 2 4 8 12  
do
    for l in 4 8 16
    do 
    ./lab2_list --threads=$t --iterations=1000 --lists=$l --sync=s >> lab2b_list.csv
    ./lab2_list --threads=$t --iterations=1000 --lists=$l --sync=m >> lab2b_list.csv
    done 
done
