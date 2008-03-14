: (                      ')' word$ drop$ ;  immediate
: \    ( -- )            0   word$ drop$ ;  immediate
: [                      state off ; immediate
: ]                      state on ;
: ,  ( x -- )            comma ;
: postpone ( -- )        ' , ; immediate
: literal                literal ;  immediate
: [']  ( -- )            ' postpone literal ; immediate
: =                      equals ;

: #  ( n1 -- n2 )        hash ;
: #s ( n1 -- n2 )        hashes ;
: <# ( -- )              <hash ;
: #> ( n -- )            hash> ;

: 0, ( -- )              0 , ;
: -rot                   minrot ;
: ?dup                   qdup ;

: recurse                latest >xt , ; immediate

: dump                   hexdump ;
: execute                exec ;
: exit,                  0, ;      immediate
: ??                     ['] branch0 ,  1 ,       ; immediate
: unless                 postpone ?? postpone exit, ; immediate


: ?comp            ( -- )          state @ unless "compilation only" error ;
: pairedwith       ( x1 x2 -- )    = unless "unmatching conditionals" error ;
: pairedwitheither ( x1 x2 x3 -- ) >r over = swap r> = or true pairedwith ;
: me         ( -- )     ?comp latest >xt  postpone literal ; immediate


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
: if     ?comp                                       ?forwards              me ; immediate
: else   ?comp      ['] if          pairedwith        forwards swap resolve me ; immediate
: endif  ?comp      ['] if ['] else pairedwitheither                resolve    ; immediate
: begin  ?comp                                                       mark   me ; immediate
: while  ?comp      ['] begin       pairedwith       ?forwards              me ; immediate
: repeat ?comp      ['] while       pairedwith  swap  backwards     resolve    ; immediate
: until  ?comp      ['] begin       pairedwith       ?backwards                ; immediate
: again  ?comp      ['] begin       pairedwith        backwards                ; immediate

: do     
    ?comp 
    ['] dodo , 
    mark  0, 
    me ;  immediate
        
: loop  ?comp 
    ['] do pairedwith 
    ['] doloop , 
    dup 2 + <resolve
    here 2 - over offset swap ! ;
; immediate

: i    ?comp ['] r@ , ; immediate


: variable create 0 , ;

