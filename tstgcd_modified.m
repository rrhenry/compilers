MODULE tstgcd;
  IMPORT In, Out;
  VAR m, n, result: INTEGER;

  PROCEDURE gcd(m, n: INTEGER): INTEGER;
  BEGIN
    WHILE m # n DO
      IF m > n THEN
        m := m - n
      ELSE
        n := n - m
      END
    END;
    RETURN m
  END gcd;

BEGIN
  
  result := gcd( m, n)

END tstgcd.



