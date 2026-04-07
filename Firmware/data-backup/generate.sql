DELETE FROM sample; DELETE FROM sqlite_sequence WHERE name='sample';

INSERT INTO sample (timestamp, sampling_time, in_flow, out_flow)
WITH RECURSIVE
  cnt(n) AS (
     SELECT 1
     UNION ALL
     SELECT n + 1 FROM cnt WHERE n < (503 * 2) -- 504 * 2 registros (7*24*3 * 2), sendo 20 minutos cada registro
  )
SELECT 
    -- 10080 é o total de minutos em 7 dias (7 * 24 * 60) e 20 minutos cada registro
    datetime('now', printf('-%d minutes', (7 * 24 * 60 * 2) - (n * 20))),
    10, 
    (ABS(RANDOM()) % 10001), 
    (ABS(RANDOM()) % 10001)
FROM cnt;