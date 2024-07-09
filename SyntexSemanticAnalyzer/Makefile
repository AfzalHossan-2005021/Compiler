make:
	bison -d 2005021.y
	flex 2005021.l
	g++ 2005021.tab.c lex.yy.c
	./a.out input.c
run:
	./a.out input.c
clean:
	rm -f a y.tab.c y.tab.h lex.yy.c y.o l.o *.o *.out *.txt *.output 2005021.tab.c 2005021.tab.h parser.c parser.h scanner.c scanner.h
