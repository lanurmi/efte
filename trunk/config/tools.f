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

: -rot                   minrot ;
: ?dup                   qdup ;

: dump                   hexdump ;
: execute                exec ;
: exit,                  0 , ;      immediate
: ?exit                  ['] branch0 ,  1 ,  0 ,  ; immediate

: ?comp            ( -- )          state @ ?exit   "compilation only" error ;
: pairedwith       ( x1 x2 -- )    = ?exit  "unmatching conditionals" error ;
: pairedwitheither ( x1 x2 x3 -- ) >r over = swap r> = or true pairedwith ;
: me         ( -- )     ?comp latest >xt  postpone literal ; immediate


\ --- flow control tool kit ---
: mark         ( -- a )        here  ;
: offset       ( a1 a2 -- n )  1 + -  ;
: condbranch   ( -- )          ['] branch0 , ;
: uncondbranch ( -- )          ['] branch  , ;
: forwards     ( -- a )        mark uncondbranch 0 , ;
: ?forwards    ( -- a )        mark   condbranch 0 , ;
: resolve      ( a -- )        1 + here over offset swap ! ;
: <resolve     ( a -- )        here offset , ;
: backwards    ( a -- )        uncondbranch <resolve  ;
: ?backwards   ( a -- )        condbranch   <resolve  ;

\ --- flow control statements ---
: if     ?comp                                       ?forwards              me ; immediate
: else   ?comp      ['] if          pairedwith        forwards swap resolve me ; immediate
: endif  ?comp      ['] if ['] else pairedwitheither                resolve    ; immediate
: begin  ?comp                                                       mark   me ; immediate
: while  ?comp      ['] begin       pairedwith       ?forwards              me ; immediate
: repeat ?comp      ['] while       pairedwith  swap  backwards     resolve    ; immediate
: until  ?comp      ['] begin       pairedwith       ?backwards                ; immediate
: again  ?comp      ['] begin       pairedwith        backwards                ; immediate
