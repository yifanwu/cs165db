USE PERFTEST;

CREATE TABLE p2test3b(
  b4a INT(8)
);

LOAD DATA LOCAL INFILE '../../cs165-tests/p2/p2table4b.csv' INTO TABLE p2test3b;
