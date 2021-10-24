# NAME: Meiyi Zheng
# EMAIL: meiyizheng@g.ucla.edu
# ID: 605147145


default:
	gcc -g -lpthread lab2_list.c SortedList.c -o lab2_list -Wall -Wextra

tests:  default
	chmod +x tests.sh
	./tests.sh 2> /dev/null

profile:
	rm -f ./profile.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./profile.gperf \
	./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./profile.gperf >> profile.out
	pprof --list=lock lab2_list profile.gperf >> profile.out
	rm -f ./profile.gperf

graphs: tests
	chmod +x lab2b_list.gp
	./lab2b_list.gp

dist:	default graphs profile
	tar -cvzf lab2b-605147145.tar.gz SortedList.h SortedList.c lab2_list.c \
	Makefile lab2b_list.csv *.png lab2b_list.gp tests.sh README profile.out

clean:
	rm -rf lab2_list *.tar.gz



