// push constant 17
@17
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 17
@17
D=A
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
// x=y
M=0
@EVAL9
D;JNE
@SP
A=M-1
M=-1
(EVAL9)
// push constant 17
@17
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 16
@16
D=A
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
// x=y
M=0
@EVAL12
D;JNE
@SP
A=M-1
M=-1
(EVAL12)
// push constant 16
@16
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 17
@17
D=A
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
// x=y
M=0
@EVAL15
D;JNE
@SP
A=M-1
M=-1
(EVAL15)
// push constant 892
@892
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 891
@891
D=A
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
// x<y
M=0
@EVAL18
D;JGE
@SP
A=M-1
M=-1
(EVAL18)
// push constant 891
@891
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 892
@892
D=A
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
// x<y
M=0
@EVAL21
D;JGE
@SP
A=M-1
M=-1
(EVAL21)
// push constant 891
@891
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 891
@891
D=A
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
// x<y
M=0
@EVAL24
D;JGE
@SP
A=M-1
M=-1
(EVAL24)
// push constant 32767
@32767
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 32766
@32766
D=A
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
// x>y
M=0
@EVAL27
D;JLE
@SP
A=M-1
M=-1
(EVAL27)
// push constant 32766
@32766
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 32767
@32767
D=A
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
// x>y
M=0
@EVAL30
D;JLE
@SP
A=M-1
M=-1
(EVAL30)
// push constant 32766
@32766
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 32766
@32766
D=A
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
// x>y
M=0
@EVAL33
D;JLE
@SP
A=M-1
M=-1
(EVAL33)
// push constant 57
@57
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 31
@31
D=A
@SP
AM=M+1
A=A-1
M=D
// push constant 53
@53
D=A
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
// push constant 112
@112
D=A
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
// -x
@SP
A=M-1
M=-M
// x&y
@SP
AM=M-1
D=M
A=A-1
MD=M&D
// push constant 82
@82
D=A
@SP
AM=M+1
A=A-1
M=D
// x|y
@SP
AM=M-1
D=M
A=A-1
MD=M|D
// !x
@SP
A=M-1
M=!M