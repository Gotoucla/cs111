# NAME: Meiyi Zheng
# EMAIL: meiyizheng@yahoo.com
# ID: 605147145

.SILENT:

default: lab0.c
	gcc -o lab0 -g -Wall -Wextra lab0.c

check: inputAndOutput inputWrong outputWrong catchAndSegfault invalidArg dumpCore

clean:
	rm -rf lab0 *.o *.txt *.tar.gz check

dist: default
	tar -czvf lab0-605147145.tar.gz lab0.c Makefile README backtrace.png breakpoint.png \

inputAndOutput:
	echo "i come from UCLA" > input.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	if [[ $$? -eq 0 ]]; then \
	echo "Success: Input file have been copied to output file"; \
	else \
	echo "Error: Option: --input --output; Input file can't be copied to output file"; \
	fi; \
	rm -rf *.txt

inputWrong:
	./lab0 --input=input.txt --output=output.txt &>/dev/null; \
	if [[ $$? -eq 2 ]]; then \
	echo "Success: Input file can't open. Exit number is 2"; \
	else \
	echo "Error: Option: --input; Exit number is not 2"; \
	fi;
	rm -rf *.txt

outputWrong:
	echo "i come from UCLA" > input.txt; \
	touch output.txt; \
	chmod u-w output.txt; \
	./lab0 --input=input.txt --output=output.txt &>/dev/null; \
	if [[ $$? -eq 3 ]]; then \
	echo "Success: Output file can't open. Exit number is 3"; \
	else \
	echo "Error: Option: --output; Exit number is not 3"; \
	fi;
	rm -rf *.txt

catchAndSegfault:
	echo "i come from UCLA" > input.txt; \
	./lab0 --input=input.txt --output=output.txt --segfault --catch &>/dev/null; \
	if [[ $$? -eq 4 ]]; then \
	echo "Success: Catch a segumentation fault. Exit number is 4"; \
	else \
	echo "Error: Option: --catch --segfault; Exit number is not 4"; \
	fi;
	rm -rf *.txt

invalidArg:
	./lab0 --k &> /dev/null; \
	if [[ $$? -eq 1 ]]; then \
	echo "Success: Invalid argument exit with number 1"; \
	else \
	echo "Error: Exit number is not 1"; \
	fi;
	rm -rf *.txt

dumpCore:
	echo "i come from UCLA" > input.txt; \
	./lab0 --input=input.txt --output=output.txt --segfault --catch --dump-core &>/dev/null; \
	if [[ $$? -eq 139 ]]; then \
	echo "Success: dump-core exit with number 139"; \
	else \
	echo "Error: Exit number is not 139"; \
	fi;
	rm -rf *.tx






