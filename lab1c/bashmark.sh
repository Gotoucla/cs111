 #!/bin/sh

#(cat pg98_100.txt | tr a-z A-Z | tr -d , | sort > out.txt) 2>> err.txt

#(cat pg98_100.txt | sort -d | grep '\<m.*s\>' | wc -c > out.txt) 2> err.txt

 (tr A-Z a-z < pg98.txt | sed 's/you/thou/' | sort -r | wc -l > out.txt) 2>err.txt

#echo $times.utime

times