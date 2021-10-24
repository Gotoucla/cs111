#! /usr/bin/gnuplot

set terminal png
set datafile separator ","

set title "List-1: Throughput of Synchronization Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/spin-lock' with linespoints lc rgb 'green'

set title "List-2: mean time per mutex wait and mean time per operation for mutex-synchronized list operations"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "mean(nanosec)"
set logscale y 10
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv"  using ($2):($8) \
	title 'mean wait time for mutex lock' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'mean time for every operation' with linespoints lc rgb 'green'

set title "List-3: number of successful iterations for each protected method"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "number of successful iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top
plot \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'yield=id, sync=s' with points lc rgb 'blue', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'yield=id, sync=m' with points lc rgb 'green', \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'yield=id' with points lc rgb 'red'

set title "List-4: throughput vs number of threads for mutex with multiple lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:14]
set ylabel "throughput"
set logscale y 10
set output 'lab2b_4.png'
set key left top
plot \
     "< grep -e 'list-none-m,[1-12]*,1000,1,' lab2b_list.csv"  using ($2):(1000000000/($7)) \
	title 'mutex list lists=1' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-m,[1-12]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'mutex list lists=4' with linespoints lc rgb 'red', \
     "<	grep -e	'list-none-m,[1-12]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
       title 'mutex list lists=8' with linespoints lc rgb 'pink', \
     "< grep -e 'list-none-m,[1-12]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
       title 'mutex list lists=16' with linespoints lc rgb 'blue' 

set title "List-5: throughput vs number of threads for spin locks with multiple lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:14]
set ylabel "throughput"
set logscale y 10
set output 'lab2b_5.png'
set key left top
plot \
     "< grep list-none-s,[1-12]*,1000,1, lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin list lists=1' with linespoints lc rgb 'green', \
     "< grep list-none-s,[1-12]*,1000,4, lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'spin list lists=4' with linespoints lc rgb 'red', \
     "< grep list-none-s,[1-12]*,1000,8, lab2b_list.csv" using ($2):(1000000000/($7)) \
       title 'spin list lists=8' with linespoints lc rgb 'pink', \
     "< grep list-none-s,[1-12]*,1000,16, lab2b_list.csv" using ($2):(1000000000/($7)) \
       title 'spin list lists=16' with linespoints lc rgb 'blue'