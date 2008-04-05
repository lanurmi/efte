: (                      ')' word$ drop$ ;  immediate
: \    ( -- )            0   word$ drop$ ;  immediate
: ,    ( x -- )          comma ;
: postpone ( -- )        ' , ; immediate
: compile ( -- )         r> count , >r ;
: literal                literal ;  immediate
: [']   ( -- )           ' postpone literal ; immediate
: =  ( x1 x2 -- f )      equals ;
: #  ( n1 -- n2 )        hash ;
: #s ( n1 -- n2 )        hashes ;
: <# ( -- )              <hash ;
: #> ( n -- )            hash> ;
: 0, ( -- )              0 , ;
: -rot ( a b c -- x a b )  minrot ;
: ?dup                   qdup ;
: recurse  ( -- )        latest >xt , ; immediate
: execute  ( a -- )      exec ;
: exit,    ( -- )        0, ;      immediate
: will     ( f -- )      ['] branch0 ,  1 ,       ; immediate
: won't    ( f -- )      ['] branch1 ,  1 ,       ; immediate
: unless   ( f -- )      postpone will  postpone exit, ; immediate
: lest     ( f -- )      postpone won't postpone exit, ; immediate
: ?comp    ( -- )        compiling unless "compilation only" error ;
: variable ( -- )        create 0, ;
: me       ( -- )        ?comp latest >xt  postpone literal ; immediate

: pairedwith       ( x1 x2 -- )    xor lest "unmatching conditionals" error ;
: pairedwitheither ( x1 x2 x3 -- ) >r over = swap r> = or true pairedwith ;

: +indent
    loading @ unless
    2 shellindent +! ;

: -indent
    loading @ unless
    -2 shellindent +!
    cursorcolumn cursorhome
    read bl = will killchar
    read bl = will killchar
    movetocolumn
;


\ --- flow control common ---
: back         ( a -- )        here - , ;
: ahead        ( -- a )        here 0, ;
: resolve      ( a -- )        here over - 1 - swap ! ;

\ --- flow control statements ---
: if  ?comp +indent
   compile branch0 ahead me
; immediate

: else   ?comp -indent +indent
   ['] if pairedwith
   compile branch ahead
   swap resolve me
; immediate

: endif  ?comp -indent
   ['] if ['] else  pairedwitheither
   resolve
; immediate

: begin  ?comp +indent
   here me
; immediate

: while  ?comp -indent +indent
   ['] begin pairedwith
   compile branch0 ahead me
; immediate

: repeat ?comp -indent
   ['] while pairedwith
   swap
   compile branch back
   resolve
; immediate

: until  ?comp -indent
   ['] begin pairedwith
   compile branch0 back
; immediate

: again  ?comp -indent
   ['] begin pairedwith
   compile branch back
; immediate

: do     ?comp +indent
   compile dodo ahead me
; immediate

: loop   ?comp -indent
   ['] do pairedwith
   dup resolve
   compile doloop 1 + back
; immediate

: (times)  ( u -- )
    r> dup 1 + >r @ >r
    0 do
       j execute
    loop r> drop ;

: times    ( u -- )
    state @ if
        ['] (times) ,
    else
       ' >r 0 do
          j execute
       loop
       r> drop
    endif ; immediate
