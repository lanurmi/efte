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
: pairedwith       ( x1 x2 -- )    = ?exit  "unpaired" error ;
: pairedwitheither ( x1 x2 x3 -- ) >r over = swap r> = or true pairedwith ;
: me         ( -- )     ?comp latest >xt  postpone literal ; immediate

\ mark a forward branch, for later resolve
: mark       ( -- a )    here  ;
: forwards   ( -- a )    mark ['] branch ,  0 , ;
: ?forwards  ( -- a )    mark ['] branch0 , 0 , ;
: resolve    ( a -- )    1 + here over 1 + - swap ! ;
: backwards  ( a -- )    ['] branch , mark 1 + - , ;
: ?backwards ( a -- )    ['] branch0 , mark 1 + - , ;


: if     ?comp                                       ?forwards              me ; immediate
: else   ?comp      ['] if          pairedwith        forwards swap resolve me ; immediate
: endif  ?comp      ['] if ['] else pairedwitheither                resolve    ; immediate
: begin  ?comp      mark me    ; immediate
: while  ?comp      ['] begin       pairedwith       ?forwards              me ; immediate
: repeat ?comp      ['] while       pairedwith  swap  backwards     resolve    ; immediate
: until  ?comp     ?backwards  ; immediate
: again  ?comp      backwards  ; immediate
