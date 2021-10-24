default:
	gcc -g lab4b.c -o lab4b -lmraa -lm -Wall -Wextra

check:	invalidOption validTest invalidCommand

clean:
	rm -rf *.tar.gz lab4b

dist:
	tar -cvzf lab4b-605147145.tar.gz lab4b.c Makefile README

invalidOption:
	./lab4b --bogus > /dev/null 2>&1; \
	if [ $$? -ne 1 ]; \
	then \
		echo "FAILED: invalid option exits with wrong code"; \
	else \
		echo "PASSED: invalid option exits with code 1"; \
	fi

validTest:
	{ echo "STOP";sleep 2; echo "START"; sleep 2; echo "SCALE=C"; sleep 2; echo "OFF"; } | ./lab4b --log=test.txt; \
	if [ $$? -ne 0 ]; \
	then \
		echo "FAILED: valid test exits with wrong code"; \
	else \
		echo "PASSED: valid test exits with code 0"; \
	fi

	for c in START STOP SCALE OFF SHUTDOWN; \
	do \
		grep "$c" test.txt > /dev/null; \
		if [ $$? -ne 0 ]; \
		then \
			echo "FAILED: command was not logged in test.txt"; \
		else \
			echo "PASSED: command was logged in test.txt"; \
		fi \
	done

	if [ ! -s test.txt ]; \
	then \
		echo "FAILED: test.txt not created"; \
	else \
		echo "PASSED: test.txt created"; \
	fi \

	rm -f test.txt

invalidCommand:
	{ echo "BOGUS"; sleep 2; echo "OFF"; } | ./lab4b --log=test.txt; \
        if [ $$? -ne 1 ]; \
        then \
                echo "FAILED: invalid command exits with wrong code"; \
        else \
                echo "PASSED: invalid command exits with code 1"; \
        fi \

	rm -f test.txt
