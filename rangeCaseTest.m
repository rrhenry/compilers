MODULE logicStructTest;
  VAR a, b, c : INTEGER;

BEGIN

  b := 4;

  a := 4;

  CASE b OF
    1..3:  a := 5 + 5;
  | 4..6:  a := 7;
  | 7..9:  a := 3;
  END;


  Out.Int( a);

END logicStrucTest.
