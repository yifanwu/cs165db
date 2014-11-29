USE CHALLENGE;
SELECT t.td, count(*)
  FROM t,u
  WHERE t.ta = u.ua
    AND t.tb = 1000
    AND u.ud BETWEEN 9000 AND 9000000
  GROUP BY t.td;
