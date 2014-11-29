USE CHALLENGE;

SELECT avg(t.td), avg(u.uc)
FROM t,u
WHERE t.ta = u.ua
        AND t.tb = 1000
        AND u.ud BETWEEN 9000 AND 9000000;
