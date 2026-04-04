INSERT INTO sample (timestamp, sampling_time, in_flow, out_flow)
WITH RECURSIVE
  cnt(n) AS (
     SELECT 1
     UNION ALL
     SELECT n + 1 FROM cnt WHERE n < 100 -- Define aqui quantas linhas quer gerar
  )
SELECT 
    -- Gera datas retroativas (ex: agora menos n segundos)
    datetime('now', '-' || (n * 10) || ' seconds'), 
    10, 
    (ABS(RANDOM()) % 2), -- Simula o (RANDOM > 0.5) gerando 0 ou 1
    (ABS(RANDOM()) % 2)
FROM cnt;