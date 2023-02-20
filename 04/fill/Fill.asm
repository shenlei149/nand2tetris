// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
    // i=SCREEN
    @SCREEN
    D=A
    @i
    M=D
(LOOP)
    // KBD!=0, goto BLACK
    @KBD
    D=M
    @BLACK
    D;JNE
    // untouched
    @i
    A=M
    M=0
    // if i != SCREEN, goto PREVIOUS
    @SCREEN
    D=A
    @i
    D=D-M
    @PREVIOUS
    D;JNE
    // goto LOOP
    @LOOP
    0;JMP
(BLACK)
    @i
    A=M
    M=-1
    // if i != KBD-1 goto NEXT
    @KBD
    D=A
    D=D-1
    @i
    D=D-M
    @NEXT
    D;JNE
    @LOOP
    0;JMP
(PREVIOUS)
    // i-=1
    @i
    M=M-1
    @LOOP
    0;JMP
(NEXT)
    // i+=1
    @i
    M=M+1
    @LOOP
    0;JMP
