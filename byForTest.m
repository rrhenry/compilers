MODULE whileTest;
  VAR a, b, c : INTEGER;

BEGIN

  b := 0;

  FOR a := 10 TO 2 BY -2 DO
    b := b + 1;
  END;

  Out.Int( b)

END whileTest.
