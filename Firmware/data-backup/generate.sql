DELETE FROM sample; DELETE FROM sqlite_sequence WHERE name='sample';

INSERT INTO sample (timestamp, sampling_time, in_flow, out_flow)
WITH RECURSIVE
  cnt(n) AS (
     SELECT 1
     UNION ALL
     SELECT n + 1 FROM cnt WHERE n < 1440
  )
SELECT 
    datetime('now', printf('-%d hours', 1440 - n)), 
    10, 
    (ABS(RANDOM()) % 10001), 
    (ABS(RANDOM()) % 10001)
FROM cnt;