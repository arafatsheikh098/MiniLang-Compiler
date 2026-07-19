CC       = gcc
CFLAGS   = -Wall -Wextra -g -std=c11
LEX_CFLAGS = -g -std=c11 -Wno-sign-compare -Wno-unused-function -Wno-comment

ifeq ($(OS),Windows_NT)
  EXEEXT := .exe
  FLEX := C:/MinGW/msys/1.0/bin/flex.exe
else
  EXEEXT :=
  FLEX := flex
endif

TARGET   = minicompiler$(EXEEXT)

LEX_OUT   = lex.yy.c
BISON_OUT = parser.tab.c parser.tab.h

SRCS = ast.c symbol_table.c semantic.c tac.c optimizer.c vm.c codegen.c main.c

OBJS = $(LEX_OUT:.c=.o)   \
       parser.tab.o        \
       ast.o               \
       symbol_table.o      \
       semantic.o          \
       tac.o               \
       optimizer.o         \
       vm.o                \
       codegen.o           \
       main.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo ""
	@echo "  ✓  Build successful → ./$(TARGET)"

$(LEX_OUT): lexer.l parser.tab.h
	$(FLEX) -t $< > $@

parser.tab.c parser.tab.h: parser.y
	bison -d -v -o parser.tab.c $<

lex.yy.o: $(LEX_OUT)
	$(CC) $(LEX_CFLAGS) -c -o $@ $<

parser.tab.o: parser.tab.c
	$(CC) $(LEX_CFLAGS) -c -o $@ $<

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

symbol_table.o: symbol_table.c symbol_table.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

semantic.o: semantic.c semantic.h symbol_table.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

tac.o: tac.c tac.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer.o: optimizer.c optimizer.h tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

vm.o: vm.c vm.h tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

codegen.o: codegen.c codegen.h tac.h vm.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c ast.h semantic.h symbol_table.h tac.h optimizer.h vm.h codegen.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET) testcases/test1.ml

clean:
	rm -f $(OBJS) $(LEX_OUT) $(BISON_OUT) parser.output $(TARGET)
	rm -f output.tac output_optimized.tac
	@echo "  ✓  Cleaned."
