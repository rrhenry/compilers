MODULE logicStructTest;
  VAR a, b, c : INTEGER;

BEGIN

  b := 0;

  REPEAT
    b := b + 1;
  UNTIL b = 5;

  Out.Int( b);

END logicStrucTest.
