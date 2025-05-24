:i count 13
:b shell 37
./deq ./examples/deque-operations.deq
:i returncode 0
:b stdout 2
4

:b stderr 0

:b shell 26
./deq ./examples/hello.deq
:i returncode 0
:b stdout 14
Hello, World!

:b stderr 0

:b shell 25
./deq ./examples/loop.deq
:i returncode 0
:b stdout 39
Loop 0
Loop 1
Loop 2
Loop 3
Loop 4
End

:b stderr 0

:b shell 25
./deq ./examples/proc.deq
:i returncode 0
:b stdout 23
Hello, EndeyshentLabs!

:b stderr 0

:b shell 25
./deq ./tests/calldir.deq
:i returncode 0
:b stdout 55
Called with right direction
Called with left direction

:b stderr 0

:b shell 34
./deq ./tests/cast-from-string.deq
:i returncode 0
:b stdout 7
2.3
69

:b stderr 0

:b shell 36
./deq ./tests/cast-string-string.deq
:i returncode 1
:b stdout 0

:b stderr 74

./tests/cast-string-string.deq:2:26: [ERR] expected an integer or a real

:b shell 22
./deq ./tests/cast.deq
:i returncode 0
:b stdout 4
2.3

:b stderr 0

:b shell 25
./deq ./tests/compare.deq
:i returncode 0
:b stdout 16
1
0
1
0
0
1
0
1

:b stderr 0

:b shell 23
./deq ./tests/deque.deq
:i returncode 0
:b stdout 7
420
68

:b stderr 0

:b shell 24
./deq ./tests/invert.deq
:i returncode 0
:b stdout 20
1
1337
69
0
1337
69

:b stderr 0

:b shell 24
./deq ./tests/labels.deq
:i returncode 0
:b stdout 10
0
1
2
3
4

:b stderr 0

:b shell 23
./deq ./tests/stack.deq
:i returncode 0
:b stdout 3
70

:b stderr 0

