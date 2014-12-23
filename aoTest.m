MODULE simpleTest;
  VAR a, b, c, d, e, f : INTEGER;

BEGIN

  a := 10 - 5;
  b := 5 - 5 - 4;

  c := ABS( a);
  d := ABS( b);

  Out.Int( c);
  Out.Int( d);

  e := ODD( c);
  f := ODD( d);

  Out.Int( e);
  Out.Int( f);

END simpleTest.
