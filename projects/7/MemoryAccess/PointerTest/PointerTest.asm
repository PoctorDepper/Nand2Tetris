// push constant 3030
@3030
D=A
@SP
AM=M+1
A=A-1
M=D
// pop pointer 0
@SP
AM=M-1
D=M
@3
M=D
// push constant 3040
@3040
D=A
@SP
AM=M+1
A=A-1
M=D
// pop pointer 1
@SP
AM=M-1
D=M
@4
M=D
// push constant 32
@32
D=A
@SP
AM=M+1
A=A-1
M=D
// pop this 2
@2
D=A
@THIS
D=D+M
@SP
AM=M-1
M=D+M
D=M-D
A=M-D
M=D
// push constant 46
@46
D=A
@SP
AM=M+1
A=A-1
M=D
// pop that 6
@6
D=A
@THAT
D=D+M
@SP
AM=M-1
M=D+M
D=M-D
A=M-D
M=D
// push pointer 0
@3
D=M
@SP
AM=M+1
A=A-1
M=D
// push pointer 1
@4
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
// push this 2
@2
D=A
@THIS
A=D+M
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
// push that 6
@6
D=A
@THAT
A=D+M
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