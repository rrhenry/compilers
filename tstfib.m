MODULE tstFib;
  VAR m: INTEGER;

  PROCEDURE fib(m: INTEGER): INTEGER;
    VAR fibVal: INTEGER;
  BEGIN
    IF m <= 1 THEN
      fibVal := 1;
    ELSE
      fibVal := m * fib( m-1);
    END;

    RETURN fibVal
  END fib;

BEGIN
 
  In.Int( m); 
 
  fib( m);

  Out.Int( m);
END tstFib.



