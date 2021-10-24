#NAME: Meiyi Zheng
#EMAIL: meiyizheng@g.ucla.edu
#ID: 605147145

default: simpsh

simpsh: simpsh.c
	gcc -g  -o simpsh simpsh.c


dist:
	tar -cvzf lab1-605147145.tar.gz simpsh.c Makefile README

clean:
	rm -rf simpsh *.tar.gz *.txt *.o

check: wait abort chdir

wait:
	touch a.txt; \
	touch b.txt; \
	./simpsh --rdonly a.txt --rdwr b.txt --pipe --command 0 1 2 bash -c "exit 10" --wait > out.txt; \
	if [[ $$? -eq 10 ]] && grep -q "exit 10 bash -c exit 10" out.txt; \
	then \
		echo "PASSED: wait command succeeds"; \
	else \
		echo "FAILED: wait command fails"; \
	fi
	rm a.txt; \
	rm b.txt; \

abort:
	./simpsh --abort; \
	if [[ $$? -eq 139 ]]; \
	then \
		echo "PASSED: abort command exits with code 139"; \
	else \
		echo "FAILED: abort command doesn't exit with code 139"; \
	fi

chdir:
	mkdir dir_test; \
	./simpsh --chdir dir_test --chdir dir_test; \
	if [[ $$? -eq 1 ]]; \
	then \
		echo "PASSED: chdir command exits with code 1 when change to the same directory"; \
	else \
		echo "FAILED: chdir command doesn't exit with code 1 when change to the same directory"; \
	fi
	rm -rf dir_test; \

