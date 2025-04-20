:i count 8
:b shell 23
./deq tests/compare.deq
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

:b shell 21
./deq tests/deque.deq
:i returncode 0
:b stdout 7
420
68

:b stderr 0

:b shell 22
./deq tests/labels.deq
:i returncode 0
:b stdout 10
0
1
2
3
4

:b stderr 0

:b shell 21
./deq tests/stack.deq
:i returncode 0
:b stdout 3
70

:b stderr 0

:b shell 35
./deq examples/deque-operations.deq
:i returncode 0
:b stdout 2
4

:b stderr 0

:b shell 24
./deq examples/hello.deq
:i returncode 0
:b stdout 14
Hello, World!

:b stderr 0

:b shell 23
./deq examples/loop.deq
:i returncode 0
:b stdout 39
Loop 0
Loop 1
Loop 2
Loop 3
Loop 4
End

:b stderr 0

:b shell 23
./deq examples/proc.deq
:i returncode 0
:b stdout 23
Hello, EndeyshentLabs!

:b stderr 0

