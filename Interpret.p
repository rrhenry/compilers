program interpret( input, output, infile);

(* Pcode interpreter  
   input: text file, 3 integers per line, separated by blanks
*)

const
  header     = 'Pcode interpreter';
  inbuffsize = 81;   (* input line length                            *)
  codesize   = 512;  (* length of code array                         *)
  addrmax    = 1023; (* maximum address                              *)
  levelmax   = 7;
   
type
  addrrange = -addrmax .. addrmax;                               
  coderange = 0 .. codesize;
  levrange  = 0 .. levelmax;
  opcodes   = ( opr, push, pshc, psha, pshi, pop, popi, jsr, isp,
                jmp, jmpc, jmpx, for1, for2, nop);

  (* instruction set                           

     opr  0, a : execute stack operation a         

                     a | operation
                    ---+-----------
                     0 | hlt - halt
                     1 | rts - return from subroutine
                     2 | neg
                     3 | add
                     4 | sub
                     5 | mul
                     6 | div
                     7 | mod
                     8 | eq
                     9 | ne
                    10 | lt
                    11 | le
                    12 | gt
                    13 | ge
                    14 | or
                    15 | and
                    16 | not
                    17 | rdi - read integer, address on TOS
                    18 | wri - write integer on TOS

     push l, a : push variable (l, a)             
     pshc 0, a : push constant a
     psha l, a : push address of variable (l, a)
     pshi 0, a : push indirect, address of variable at (l, a)
     pop  l, a : pop into variable (l, a)
     popi l, a : pop indirect, address of variable at (l, a)
     jsr  l, a : call procedure a l levels up 
     isp  0, a : increment stack pointer by a  
     jmp  0, a : jump to address a                     
     jmpc l, a : jump conditional to address a ( l = 0: false; l = 1: true)
     jmpx 0, a : jump indexed
     for1 l, a : jump to address a if zero trips (l = 0: to; l = 1: downto)
     for2 l, a : jump to address a if repeated trip
  *)

  vminstr = packed record
              op: opcodes;  (* operation code                           *)
              ld: levrange; (* static level difference                  *)
              ad: addrrange (* relative displacement within stack frame *)
            end;
  codearray = packed array[ coderange] of vminstr;

var
  ch:         char;    (* last character read from source file *)
  inbuff:     array[ 1 ..inbuffsize] of char; (* current line  *)
  infile:     text;
  nl:         char; (* new line character *)
   

  mnemonic: array[ opcodes] 
              of packed array[ 1 .. 4] of char;

  code: codearray;        (* storage for machine instructions *)
  lc:   coderange;        (* location counter                 *)
  oplist: array[ 0 .. 20] of opcodes;
  digits,
  letters,
  separators: set of char;

(* ********************* interpreter initialization **********************)

  procedure initinterp;
    
    procedure initscalars;       
    begin (* initscalars *)
      nl     := chr( 10);
    end; (* initscalars *)

    procedure initsets;
    begin (* initsets *)
      digits          := [ '0' .. '9'];
      letters         := [ 'a' .. 'z', 'A' .. 'Z'];
      separators      := [' ', nl];
    end; (* initsets *)
 
    procedure initfiles;
    const
      filenmsize = 20;
    var
      filename:   packed array[ 1.. filenmsize] of char;
      i: integer;
    begin (* initfiles *)
      writeln;
      write( 'sourcefile: ');
      i := 1;
      while (not eoln) and (i <= filenmsize) do
        begin
          read( ch);
          filename[ i] := ch;
          i := i + 1
        end;
      writeln;
      assign( infile, filename);
      reset( infile)        
    end; (* initfiles *)

    procedure initinstrmnemonics;
    begin
      mnemonic[ opr ] := 'opr ';
      mnemonic[ push] := 'push';
      mnemonic[ pshc] := 'pshc';
      mnemonic[ psha] := 'psha';
      mnemonic[ pshi] := 'pshi';
      mnemonic[ pop ] := 'pop ';
      mnemonic[ popi] := 'popi';
      mnemonic[ jsr ] := 'jsr ';
      mnemonic[ isp ] := 'isp ';
      mnemonic[ jmp ] := 'jmp ';
      mnemonic[ jmpc] := 'jmpc';
      mnemonic[ jmpx] := 'jmpx';
      mnemonic[ for1] := 'for1';
      mnemonic[ for2] := 'for2';
      mnemonic[ nop ] := 'nop '
    end; (* initinstrmnemonics *)

    procedure initoplist;
    begin
      oplist[  0] := opr;
      oplist[  1] := push;
      oplist[  2] := pshc;
      oplist[  3] := psha;
      oplist[  4] := pshi;
      oplist[  5] := pop;
      oplist[  6] := popi;
      oplist[  7] := jsr;
      oplist[  8] := isp;
      oplist[  9] := jmp;
      oplist[ 10] := jmpc;
      oplist[ 11] := jmpx;
      oplist[ 12] := for1;
      oplist[ 13] := for2;
      oplist[ 14] := nop
    end;

    procedure inputcode;
    var
      n: integer;
      inptr: integer; 

      procedure  getline;
      begin (* getline *)
        inptr := 0;
        while not eoln( infile) do
          begin
            inptr := inptr + 1;
            read( infile, ch);
            inbuff[ inptr] := ch
          end;
        inptr := inptr + 1;
        read( infile, ch); (* read eoln char; ch contains a blank *)
        inbuff[ inptr] := nl; (* store nl character in inbuff     *)
        inptr := 0
      end; (* getline *)
   
      procedure nextchar;
      begin (* nextchar *)
        inptr := inptr + 1;
        ch := inbuff[ inptr]
      end; (* nextchar *)
      
      procedure skipsep;
      begin (* skipsep *)
        while ch in separators do
          nextchar
      end; (* skipsep *)
   
      procedure getnum( var i: integer);
      var
        sign: integer;
      begin (* getnum *)
        skipsep;
        sign := 1;
        if ch = '-' then
          begin
            sign := -1;
            nextchar
          end;
        i := 0;
        repeat
          i := 10 * i + ord( ch) - ord( '0');
          nextchar
        until not (ch in digits);
        i := sign * i
      end; (* getnum *)
       
    begin (* inputcode *)
      lc := 0;
      while not eof( infile) do
        begin
          getline;
          getnum( n);
          with code[ lc] do
            begin
              op := oplist[ n];
              getnum( n);
	      ld := n;
              getnum( n);
              ad := n
            end;
	  with code[ lc] do
            writeln( lc: 3,': ', mnemonic[ op], ' ', ld,' ', ad);
          lc := lc + 1
        end
    end; (* inputcode *)

  begin (* initinterp *)
    writeln;
    writeln( header);
    initscalars;
    initsets;
    initfiles;
    initinstrmnemonics;
    initoplist;
    inputcode
  end; (* initinterp *)

(* ****************** end of interpreter initialization ******************** *)

(* ********************* intermediate code interpreter ********************* *)

  procedure interp;
  const
    stkmax = 1023; (* maximum size of data store *)
  type
    address = 0 .. stkmax;
  var
    ip: coderange;                    (* instruction  pointer                *)
    bp,                               (* frame pointer                       *)
    sp: address;                      (* top of stack pointer                *)
    ir: vminstr;                      (* instruction buffer                  *)
    ps: ( run, stop, divchk, stkchk,
          caschk);                    (* processor status register           *)
    xstack: array[ address] of        (* run time stack                      *)
                 integer;                                            
   
  (* stack structure                                             
   
       |                | <--- last element in previous stack frame
       +----------------+                                        
       |  func ret val  |
       +----------------+
       |   parameters   |
       |                |
       +----------------+
       |  static link   |                     
       +----------------+ |    direction of growth
       | return address | V                                       
       +----------------+ 
       |  dynamic link  | <--- base pointer                            
       | (old base ptr) |
       +----------------+ 
       |     local      |                                        
       |   variables    |
       +----------------+
       |  temporaries   | <--- stack pointer
  *)

    opcount: integer; (* number of operations                                *)
    t1: integer;
    t2: address;
    xf: 0 .. 1; (* loop exit flag *)
                    
    procedure inter;
     
    function fbase( levdiff: levrange) :address;
    (* find frame base levdiff levels down on static chain                   *)
    var
      tmpptr: address;
    begin (* fbase *)
      tmpptr := bp - 2;
      while levdiff > 0 do
        begin
          tmpptr := xstack[ tmpptr];
          levdiff := levdiff - 1
        end;
      fbase := tmpptr +2
    end; (* fbase *)

    procedure pushac;
    (* push new activation record on stack                                   *)
    begin (* pushac *)
      sp := sp + 1;
      xstack[ sp] := fbase( ir.ld) - 2;      (* store static link            *)
      xstack[ sp + 1] := ip;                 (* store return address         *)
      xstack[ sp + 2] := bp;                 (* store dynamic link           *)
      sp := sp + 2;
      bp := sp                               (* new frame pointer            *)
    end; (* pushac *)

    procedure popac;
    (* pop activation record off stack *)
    begin (* popac *)
      sp := bp;              (* set sp to base of current frame              *)
      ip := xstack[ sp - 1]; (* restore previous ip                          *)
      bp := xstack [ sp];    (* get bp for previous frame                    *)
      sp := sp - 3           (* restore sp to top of previous frame          *)
    end; (* popac *)
   
  begin (* inter *)
    with ir do
      case op of
        opr : case ad of
                 0: (* hlt - processar halt *)
                    ps := stop;
                 1: (* rts - proc return, pop activation record off stack    *)
                    popac;
                    (* stack operations *)
                 2: (* neg *)
                    xstack[ sp] := - xstack[ sp];
                 3: begin (* add *)
                      sp := sp - 1;
                      xstack[ sp] := xstack[ sp] + xstack[ sp + 1]
                    end;
                 4: begin (* sub *)
                      sp := sp - 1;
                      xstack[ sp] := xstack[ sp] - xstack[ sp + 1]
                    end;
                 5: begin (* mul *)
                      sp := sp - 1;
                      xstack[ sp] := xstack[ sp] * xstack[ sp + 1]
                    end;
                 6: begin (* div *)
                      sp := sp - 1;
                      if xstack[ sp + 1] = 0 then 
                        ps := divchk
                      else
                        xstack[ sp] := xstack[ sp] div xstack[ sp + 1]
                    end;
                 7: begin (* mod *)
                      sp := sp - 1;
                      if xstack[ sp + 1] = 0 then
                        ps := divchk
                      else
                        xstack[ sp ] := xstack[ sp] mod xstack[ sp + 1]
                    end;
                 8: begin (* eq *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] = xstack[ sp + 1])
                    end;
                 9: begin (* ne *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] <> xstack[ sp + 1])
                    end;
                10: begin (* lt *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] < xstack[ sp + 1])
                    end;
                11: begin (* le *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] <= xstack[ sp + 1])
                    end;
                12: begin (* gt *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] > xstack[ sp + 1])
                    end;
                13: begin (* ge *)
                      sp := sp - 1;
                      xstack[ sp] := ord( xstack[ sp] >= xstack[ sp + 1])
                    end;
                14: begin (* or *)
                      sp := sp - 1;
                      if xstack[ sp + 1] = 1 then
                        xstack[ sp] := 1
                    end;
                15: begin (* and *)
                      sp := sp - 1;
                      if xstack[ sp + 1] = 0 then
                        xstack[ sp] := 0
                    end;
                16: (* not *)
                    xstack[ sp] := 1 - xstack[ sp];

                17: begin (* rdi - read integer, address on TOS *)
                      writeln;
                      write( 'Program Input: ');
                      readln( xstack[ xstack[ sp]]);
                      sp := sp - 1
                    end;			
                18: begin (* wri - write integer on TOS         *)
                      writeln; 
		      write( 'Program Output: ');
		      writeln( xstack[ sp]);
		      sp := sp -1
                    end
              end; (* case ad *)

        push: begin  
                sp := sp + 1;
                if sp > stkmax then
                  ps := stkchk
                else
                  xstack[ sp] := xstack[ fbase( ld) + ad]
              end;
        pshc: begin (* push constant *)
                sp := sp + 1;
                if sp > stkmax then
                  ps := stkchk
                else
                  xstack[ sp] := ad
                end;
        psha: begin (* push address *)
                sp := sp + 1;
                if sp > stkmax then
                  ps := stkchk
                else
                  xstack[ sp] := fbase( ld) + ad
              end;
        pshi: begin (* push indirect *)
                sp := sp + 1;
                if sp > stkmax then
                  ps := stkchk
                else
                  xstack[ sp] := xstack[ xstack[ fbase( ld) + ad] ]
              end;
        pop:  begin
                xstack[ fbase( ld) + ad] := xstack[ sp];
                sp := sp - 1
              end;
        popi: begin (* pop indirect *)
                xstack[ xstack[ fbase( ld) + ad] ] := xstack[ sp];
                sp := sp - 1
              end;
        jsr:  (* jump to subroutine *)
              if sp + 3 > stkmax then
                ps := stkchk
              else
                begin
                  pushac;  (* push new activation record onto stack          *)
                  ip := ad (* set ip to entry point of proc                  *)
                end;
        isp:  (* increment stack pointer - allocate stack space              *)
              if sp + ad > stkmax then
                ps := stkchk
              else
                sp := sp + ad;
        jmp:  ip := ad;
        jmpc: begin (* jump on condition                                     *)
                if xstack[ sp] =  ld then
                  ip := ad;
                sp := sp - 1
              end;
        jmpx: begin (* jump indexed                                          *)
                t1 := xstack[ sp];
                sp := sp - 1;
                t2 := ad;
                xf := 0;
                repeat
                  if code[ t2].op <> nop then
                    begin
                      xf := 1;
                      ps := caschk
                    end
                  else
                    if code[ t2]. ad = t1 then
                      begin
                        xf := 1;
                        ip := code[ t2 + 1].ad
                      end
                    else
                      t2 := t2 + 1
                until xf = 1
              end;
        for1: begin (* for loop zero trips check                             *)
                t1 := xstack[ sp - 1];
                xstack[ xstack[ sp - 2]] := t1;
                if ld = 0 then
                  begin
                    if t1 > xstack[ sp] then
                      begin
                        ip := ad;
                        sp := sp - 3
                      end
                  end
                else
                  if t1 < xstack[ sp] then
                    begin
                      ip := ad;
                      sp := sp - 3
                    end                                 
              end;
        for2: begin (* for loop repeat trip check                            *)
                t2 := xstack[ sp - 2];
                t1 := xstack[ t2];
                if ld = 0 then
                  begin
                    if t1 < xstack[ sp] then
                      begin
                        xstack[ t2] := t1 + 1;
                        ip := ad
                      end
                    else
                      sp := sp - 3
                  end
                else
                  begin
                    if t1 > xstack[ sp] then
                      begin
                        xstack[ t2] := t1 - 1;
                        ip := ad
                      end
                    else
                      sp := sp - 3
                  end
              end
      end (* case op *)
    end; (* inter *)

    procedure liststack;
    var
      isp: address;
    begin (* liststack *)
      writeln;
      for isp := 0 to sp do
        writeln( isp: 3, ': ', xstack[ isp])
    end; (* liststack *)
  
    procedure pmdump;
    begin (* pmdump *)
      writeln;
      write( 'processor halt at ip = ', ip: 5, ' due to ');
      case ps of
        divchk: writeln( 'division by zero');
        stkchk: writeln( 'run-time stack overflow')
      end (* case *)
    end; (* pmdump *)
   
  begin (* interp *)
    writeln;
    writeln( 'Oberon-S execution');
    (* initialize stack    *)
    xstack[ 0] := 0;                    (*        +---------+        *)
    xstack[ 1] := 0;                    (*        |    0    |        *)
    xstack[ 2] := 0;                    (*        +---------+        *)
    sp := 2;                            (*        |    0    |        *)
    bp := 2;                            (*        +---------+        *)
    opcount := 0;                       (*  sp -> |    0    | <- bp  *)
    ps := run;                          (*        +---------+        *)
    ip := 0;
    repeat 
      ir := code[ ip]; (* fetch instruction   *)
      ip := ip + 1;
      opcount := opcount + 1;
      inter;           (* execute instruction *)
      liststack;
    until ps <> run;
    if ps <> stop then
      pmdump
    else
      begin
        writeln;
        writeln( 'End of execution');
        writeln( opcount: 6, ' operations')
      end
  end; (* interp *)

begin (* interpret *)
  initinterp;
  interp 
end. (* interpret *)         
