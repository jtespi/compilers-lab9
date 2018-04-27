# Lab 9 makefile
#
#   produces an executable, lab9
#

# Jacob Espinoza
# CS 370 - Compilers
# 2018 May 04

all: lab9

lab9: lab9.l lab9.y emitAST.c AST.c AST.h symtable.c symtable.h
	yacc -d lab9.y
	lex lab9.l
	gcc y.tab.c lex.yy.c emitAST.c AST.c symtable.c -o lab9
