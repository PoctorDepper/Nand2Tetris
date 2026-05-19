// push constant 111
@111
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 333
@333
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 888
@888
D=A
@SP
AM=M+1
A=A-1
M=D
// pop static 8
@SP
AM=M-1
D=M
@24
M=D
// pop static 3
@SP
AM=M-1
D=M
@19
M=D
// pop static 1
@SP
AM=M-1
D=M
@17
M=D
// push static 3
@19
D=M
@SP
AM=M+1
A=A-1
M=D
// push static 1
@17
D=M
@SP
AM=M+1
A=A-1
M=D
// x-y
@SP
AM=M-1
D=M
A=A-1
MD=M-D
// push static 8
@24
D=M
@SP
AM=M+1
A=A-1
M=D
// x+y
@SP
AM=M-1
D=M
A=A-1
MD=M+D