CREATE DATABASE PERFTEST;
USE PERFTEST;

DROP TABLE p2test3;
-- must delete and create to simulate time used
CREATE TABLE p2test3(
  t4a INT(8),
  t4b INT(8),
  t4c INT(8),
  t4d INT(8),
  t4e INT(8),
  t4f INT(8),
  t4g INT(8),
  t4h INT(8),
  t4i INT(8),
  t4j INT(8)
);

LOAD DATA LOCAL INFILE '../../cs165-tests/p2/p2table4.csv' INTO TABLE p2test3 FIELDS TERMINATED BY ',';
