CC = gcc
CFLAGS = -g
DEST = compile

HFILES = error.h  global.h  protos.h symbol-table.h  syntax-tree.h

CFILES = error.c \
	lex.yy.c \
	main.c\
	print.c \
	symbol-table.c\
    syntax-tree.c \
	util.c\
	y.tab.c\
	instruction.c\
	code_generation.c\
	code_translation.c

OFILES = error.o \
	lex.yy.o \
	main.o \
	print.o \
	symbol-table.o \
    syntax-tree.o \
	util.o \
	y.tab.o\
	instruction.o\
    code_generation.o\
    code_translation.o

.c.o :
	$(CC) $(CFLAGS) -c $<

$(DEST) : $(OFILES)
	$(CC) -o $(DEST) $(OFILES) -ll

error.o : error.h global.h syntax-tree.h error.c y.tab.h

main.o : global.h main.c code_translation.c

symbol-table.o : global.h symbol-table.h symbol-table.c

syntax-tree.o : global.h instruction.h syntax-tree.h syntax-tree.c

instruction.o : global.h symbol-table.c instruction.c

code_generation.o : syntax-tree.c protos.h instruction.c

code_translation.o : syntax-tree.c protos.h instruction.c

util.o : global.h util.h util.c

lex.yy.o : global.h error.h syntax-tree.h symbol-table.h lex.yy.c

y.tab.c : parser.y 
	yacc -d -v parser.y

lex.yy.c : y.tab.h scanner.l 
	flex scanner.l

y.tab.h : parser.y
	yacc -d -v parser.y

clean :
	/bin/rm -f *.o core *.BAK

realclean :
	/bin/rm -f *.o core *.BAK lex.yy.c y.tab.c y.tab.h y.output compile
