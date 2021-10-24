#NAME: Meiyi Zheng
#EMAIL: meiyizheng@g.ucla.edu
#ID: 605147145

default: simpsh

simpsh: simpsh.c
	gcc -g -std=c11 -Wall -o simpsh simpsh.c


dist:
	tar -cvzf lab1-605147145.tar.gz simpsh.c Makefile README

clean:
	rm -rf simpsh *.tar.gz *.txt *.o

check: readAndWrite command verbose

readAndWrite:
	touch a.txt; \
	touch b.txt; \
	touch c.txt; \
	./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt; \
	if [[ $$? -eq 0 ]]; then \
	echo "SUCCESS: The readAndWrite test exits with code 0"; \
	else \
	echo "FAIL: The readAndWrite test should exit with code 0"; \
	fi;
	rm -rf *.txt

command:
	touch a.txt; \
	touch b.txt; \
	touch c.txt; \
	./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 1 2 cat; \
	if [[ $$? -eq 0 ]] ; then \
	echo "SUCCESS: The command test exits with code 0"; \
	else \
	echo "FAIL: The command test should exit with code 0"; \
	fi;
	rm -rf *.txt

verbose:
	touch a.txt; \
	touch b.txt; \
	./simpsh --verbose --rdonly a.txt > b.txt; \
	grep -q -- "--rdonly a.txt" b.txt; \
	if [[ $$? -eq 0 ]] ; then \
	echo "SUCCESS: The verbose test output all options and arguments behind it"; \
	else \
	echo "FAIL: The verbose test doesn't output all options and arguments behind it"; \
	fi;
	rm -rf *.txt
