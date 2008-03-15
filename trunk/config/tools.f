: (                      ')' word$ drop$ ;  immediate
: \    ( -- )            0   word$ drop$ ;  immediate
: [    ( -- )            state off ; immediate
: ]    ( -- )            state on ;
: ,    ( x -- )          comma ;
: postpone ( -- )        ' , ; immediate
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
: ??       ( f -- )      ['] branch0 ,  1 ,       ; immediate
: unless   ( f -- )      postpone ?? postpone exit, ; immediate
: ?comp    ( -- )        state @ unless "compilation only" error ;
: variable ( -- )        create 0, ;
: me       ( -- )        ?comp latest >xt  postpone literal ; immediate

: pairedwith       ( x1 x2 -- )    = unless "unmatching conditionals" error ;
: pairedwitheither ( x1 x2 x3 -- ) >r over = swap r> = or true pairedwith ;

: +indent  2 shellindent +! ;

: -indent -2 shellindent +!    \ a bit unwieldy without loops yet.
    cursorcol cursorhome
    read bl = ?? killchar
    read bl = ?? killchar
    movetocolumn    
;


\ --- flow control tool kit ---
: mark         ( -- a )        here  ;
: offset       ( a1 a2 -- n )  1 + -  ;
: branch,      ( -- )          ['] branch , ;
: branch0,     ( -- )          ['] branch0  , ;
: forwards     ( -- a )        mark  branch,  0, ;
: ?forwards    ( -- a )        mark  branch0, 0, ;
: resolve      ( a -- )        1 + here over offset swap ! ;
: <resolve     ( a -- )        here offset , ;
: backwards    ( a -- )        branch,  <resolve  ;
: ?backwards   ( a -- )        branch0, <resolve  ;

\ --- flow control statements ---
: if     ?comp  +indent                                          ?forwards              me ; immediate
: else   ?comp  -indent +indent ['] if          pairedwith        forwards swap resolve me ; immediate
: endif  ?comp  -indent         ['] if ['] else pairedwitheither                resolve    ; immediate
: begin  ?comp  +indent                                                          mark   me ; immediate
: while  ?comp  -indent +indent ['] begin       pairedwith       ?forwards              me ; immediate
: repeat ?comp  -indent         ['] while       pairedwith  swap  backwards     resolve    ; immediate
: until  ?comp  -indent         ['] begin       pairedwith       ?backwards                ; immediate
: again  ?comp  -indent         ['] begin       pairedwith        backwards                ; immediate

: do
    ?comp 
    +indent
    ['] dodo , 
    mark  0, 
    me ;  immediate
        
: loop  ?comp 
    -indent
    ['] do pairedwith 
    ['] doloop , 
    dup 2 + <resolve
    here 2 - over offset swap ! ;
; immediate
