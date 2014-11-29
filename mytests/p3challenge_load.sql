CREATE DATABASE CHALLENGE;
USE CHALLENGE;
CREATE TABLE t(
  ta INT(8),
  tb INT(8),
  tc INT(8),
  td INT(8)
);

LOAD DATA LOCAL INFILE '../../cs165-tests/p3challenge/t.csv' INTO TABLE t FIELDS TERMINATED BY ',';


CREATE TABLE u(
  ua INT(8),
  ub INT(8),
  uc INT(8),
  ud INT(8)
);

LOAD DATA LOCAL INFILE '../../cs165-tests/p3challenge/u.csv' INTO TABLE u FIELDS TERMINATED BY ',';
