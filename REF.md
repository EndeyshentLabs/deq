# Language reference

## Directional
- `drop` ( a -- ) -- remove the top element
- `dup` ( a -- a a ) -- duplicate the top element
- `swap` ( a b -- b a ) -- swap 2 elements
- `move` ( a -- a ) -- move element to other side of the deque
- `rot` ( a b c -- c b a ) -- rotate 3 elements
- `over` ( a b -- a b a ) -- duplicate the element below the top of the deque
- `add` ( a b -- a+b ) -- sum up top-2 elements of the deque
- `mul` ( a b -- a*b ) -- multiply top-2 elements of the deque
- `sub` ( a b -- a-b ) -- subtract the top element of the deque from the one below it
- `div` ( a b -- a/b ) -- divide the element below the top of the deque by the top one
- `mod` ( a b -- a%b ) -- modulo, order of parameters as in `sub` or `div`
- `shr` ( a b -- a>>b ) -- (bitwise) SHIFT to the RIGHT the top element of the deque by the amount of the element below it
- `shl` ( a b -- a<<b ) -- (bitwise) SHIFT to the LEFT the top element of the deque by the amount of the element below it
- `band` ( a b -- a&b ) -- (bitwise) AND top-2 elements
- `bor` ( a b -- a|b ) -- (bitwise) OR top-2 elements
- `bnot` ( a -- ~a ) -- (bitwise) NOT the top element
- `eq` ( a b -- a==b ) -- (logical) equality
- `neq` ( a b -- a!=b ) -- (logical) not-equality
- `lt` ( a b -- a<b ) -- less-than
- `lteq` ( a b -- a<=b ) -- less-or-equal-than
- `gt` ( a b -- a>b ) -- greater-than
- `gteq` ( a b -- a>=b ) -- greater-or-equal-than
- `and` ( a b -- a&&b ) -- (logical) AND
- `or` ( a b -- a||b ) -- (logical) OR
- `not` ( a -- !a ) -- (logical) NOT
- `jmp` ( addr -- ) -- unconditional jump to label `addr`
- `call` ( addr -- ) -- call to label `addr`. Return from call is performed using `ret` keyword.
- `jz` ( cond addr -- ) -- jump to label `addr` if cond==0
- `jnz` ( cond addr -- ) -- jump to label `addr` if cond!=0
- `print` ( a -- ) -- print any element (works with all types)
- `println` ( a -- ) -- `print`, but also prints life-feed (aka new-line) at the end
- `putc` ( c -- ) -- prints `c` as character to stdout
- `calldir` ( -- dir ) -- pushes 1 if label was called with left direction, 0 otherwise
- `invertdir` ( -- newinverted ) -- pushes new value of `inverted` flag and flips it effectively inverting directions of all subsequent operations
- `setinverted` ( invertflag -- ) -- sets value of `inverted` flag
- `>real` ( int|string -- real ) -- casts integer or string to real
- `>integer` ( real|string -- int ) -- casts real or string to integer
- `>string` ( int|real -- string ) -- casts integer or real to string

## Not directional
- `trace` -- print current deque state
- `ret` -- return from call
- `exit` -- halt execution
