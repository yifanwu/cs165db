#rm db_soc
./client < ../cs165-tests/p2/p2test1a.txt > ../cs165-tests/p2/p2test1a.result
diff ../cs165-tests/p2/p2test1a.result ../cs165-tests/p2/p2test1a.expected
./client < ../cs165-tests/p2/p2test1b.txt > ../cs165-tests/p2/p2test1b.result
diff ../cs165-tests/p2/p2test1b.result ../cs165-tests/p2/p2test1b.expected
./client < ../cs165-tests/p2/p2test1c.txt > ../cs165-tests/p2/p2test1c.result
diff ../cs165-tests/p2/p2test1c.result ../cs165-tests/p2/p2test1c.expected
./client < ../cs165-tests/p2/p2test2.txt > ../cs165-tests/p2/p2test2.result
diff ../cs165-tests/p2/p2test2.result ../cs165-tests/p2/p2test2.expected
./client < ../cs165-tests/p2/p2test3.txt > ../cs165-tests/p2/p2test3.result
diff ../cs165-tests/p2/p2test3.result ../cs165-tests/p2/p2test3.expected
./client < ../cs165-tests/p2/p2test4.txt > ../cs165-tests/p2/p2test4.result
diff ../cs165-tests/p2/p2test4.result ../cs165-tests/p2/p2test4.expected
./client < ../cs165-tests/p2/p2test5.txt > ../cs165-tests/p2/p2test5.result
diff ../cs165-tests/p2/p2test5.result ../cs165-tests/p2/p2test5.expected
./client < ../cs165-tests/p2/p2test6.txt > ../cs165-tests/p2/p2test6.result
diff ../cs165-tests/p2/p2test6.result ../cs165-tests/p2/p2test6.expected
