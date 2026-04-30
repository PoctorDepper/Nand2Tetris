// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

// Initialize key value to no key
@key
M=0

// Has the input changed?
(INPUT)
  @key
  D=M
  @KBD
  D=D-M
  @INPUT
  D;JEQ

// Set both D and @key to current key
@key
MD=M-D

// Should we blank or draw?
@BLANK
D;JEQ
  D=-1
(BLANK)
@draw
M=D

// Set end point
@8191 // 8k-1
D=A
@SCREEN
D=D+A
@FILL
M=D

// Set start point
@SCREEN
D=A-1

// Fill the screen with @draw
(FILL)
  // Adjust screen block pointer
  @SCREEN
  A=A-1
  M=D+1
 
  // Grab the draw value
  @draw
  D=M
  @SCREEN
  A=A-1
  A=M
  M=D

  // Check end point
  D=A
  @FILL
  M-D;JGT

@INPUT
0;JMP
