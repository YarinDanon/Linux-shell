# Linux-shell
Linux-shell this is a terminal (shell/bash) simulator with linux command

support (without folder control) and "@" , "|" , "<" , ">" , ">>" and "2>" argumenet support.

There can be up to 2 processes running per client (one for command, one for shell/bash CLI). there is a set limit of an input up to 511 
characters and a limit of hostname display of 511 characters

how to install: open linux terminal, navigate to the folder containing ex2.c using the "cd" command (confirm it by using ls command) 
incase you have makefile, type make and the program will automaticily be compiled, if you don't, type gcc -Wall ex2.c -o ex2 and your 
program will automaticily be compiled

run the program: open linux terminal, navigate to ex2 executeable file location using "cd" command (confirm it using ls command) and type 
./ex2

to operate: once you are inside ex2 type any command you want

*incase a command execution was successfull, return code will be updated

*incase the command "done" was sent, the terminal simulator will exit

*incase of empty command input(space enter, enter...) or any failure to fetch input from user, last return code will be retained and a new line will be made for the simulator command
