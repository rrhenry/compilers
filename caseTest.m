MODULE logicStructTest;
  VAR a, b, c : INTEGER;

BEGIN

  b := 3;

  a := 4;

  CASE b OF
    1:  a := 5 + 5;
  | 2:  a := 7;
  | 3:  a := 3; 
  END;


  Out.Int( a);

END logicStrucTest.
