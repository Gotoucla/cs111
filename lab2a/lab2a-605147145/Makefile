# NAME: Meiyi Zheng
# EMAIL: meiyizheng@g.ucla.edu
# ID: 605147145


build:
	gcc -g lab2_add.c -o lab2_add -lpthread -Wall -Wextra
	gcc -g -lpthread lab2_list.c SortedList.c -o lab2_list -Wall -Wextra

tests:  build
	chmod +x tests.sh
	./tests.sh 2> /dev/null

graphs: tests
	chmod +x lab2_add.gp
	chmod +x lab2_list.gp
	./lab2_add.gp
	./lab2_list.gp

dist:	build graphs
	tar -cvzf lab2a-605147145.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c \
	Makefile lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png \
	lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.gp lab2_list.gp tests.sh README

clean:
	rm -rf lab2_add lab2_list *.tar.gz



