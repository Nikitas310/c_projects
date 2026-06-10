CC = gcc
CFLAGS = -ansi -Wall -pedantic
TARGET = assembler
OBJS = main.o assembler.o preassembler.o parser.o first_pass.o second_pass.o output_files.o data_image.o code_image.o external_usage.o utils.o symbol_table.o macro_table.o opcode_table.o errors.o
TEST_TARGET = test_phase1
TEST_OBJS = test_phase1.o utils.o symbol_table.o macro_table.o opcode_table.o errors.o
TEST_PHASE2_TARGET = test_phase2
TEST_PHASE2_OBJS = test_phase2.o preassembler.o utils.o symbol_table.o macro_table.o opcode_table.o errors.o
TEST_PARSER_TARGET = test_parser
TEST_PARSER_OBJS = test_parser.o parser.o utils.o opcode_table.o errors.o
TEST_FIRST_PASS_TARGET = test_first_pass_directives
TEST_FIRST_PASS_OBJS = test_first_pass_directives.o first_pass.o data_image.o code_image.o external_usage.o parser.o utils.o symbol_table.o opcode_table.o errors.o
TEST_FIRST_PASS_INSTRUCTIONS_TARGET = test_first_pass_instructions
TEST_FIRST_PASS_INSTRUCTIONS_OBJS = test_first_pass_instructions.o first_pass.o data_image.o code_image.o external_usage.o parser.o utils.o symbol_table.o opcode_table.o errors.o
TEST_SECOND_PASS_TARGET = test_second_pass
TEST_SECOND_PASS_OBJS = test_second_pass.o second_pass.o first_pass.o data_image.o code_image.o external_usage.o parser.o utils.o symbol_table.o opcode_table.o errors.o
TEST_OUTPUT_TARGET = test_output_files
TEST_OUTPUT_OBJS = test_output_files.o output_files.o second_pass.o first_pass.o data_image.o code_image.o external_usage.o parser.o utils.o symbol_table.o opcode_table.o errors.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c assembler_module.h
	$(CC) $(CFLAGS) -c main.c

assembler.o: assembler.c assembler_module.h preassembler.h first_pass.h second_pass.h output_files.h
	$(CC) $(CFLAGS) -c assembler.c

preassembler.o: preassembler.c preassembler.h assembler.h macro_table.h utils.h errors.h opcode_table.h
	$(CC) $(CFLAGS) -c preassembler.c

parser.o: parser.c parser.h assembler.h utils.h errors.h
	$(CC) $(CFLAGS) -c parser.c

first_pass.o: first_pass.c first_pass.h assembler.h parser.h symbol_table.h data_image.h utils.h errors.h
	$(CC) $(CFLAGS) -c first_pass.c

second_pass.o: second_pass.c second_pass.h first_pass.h assembler.h parser.h utils.h errors.h
	$(CC) $(CFLAGS) -c second_pass.c

output_files.o: output_files.c output_files.h first_pass.h assembler.h utils.h
	$(CC) $(CFLAGS) -c output_files.c

data_image.o: data_image.c data_image.h assembler.h
	$(CC) $(CFLAGS) -c data_image.c

code_image.o: code_image.c code_image.h assembler.h
	$(CC) $(CFLAGS) -c code_image.c

external_usage.o: external_usage.c external_usage.h assembler.h
	$(CC) $(CFLAGS) -c external_usage.c

utils.o: utils.c utils.h assembler.h opcode_table.h
	$(CC) $(CFLAGS) -c utils.c

symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

macro_table.o: macro_table.c macro_table.h
	$(CC) $(CFLAGS) -c macro_table.c

opcode_table.o: opcode_table.c opcode_table.h
	$(CC) $(CFLAGS) -c opcode_table.c

errors.o: errors.c errors.h
	$(CC) $(CFLAGS) -c errors.c

test_phase1.o: test_phase1.c assembler.h symbol_table.h macro_table.h opcode_table.h utils.h errors.h
	$(CC) $(CFLAGS) -c test_phase1.c

test_phase2.o: test_phase2.c preassembler.h assembler.h
	$(CC) $(CFLAGS) -c test_phase2.c

test_parser.o: test_parser.c parser.h assembler.h
	$(CC) $(CFLAGS) -c test_parser.c

test_first_pass_directives.o: test_first_pass_directives.c first_pass.h assembler.h symbol_table.h data_image.h
	$(CC) $(CFLAGS) -c test_first_pass_directives.c

test_first_pass_instructions.o: test_first_pass_instructions.c first_pass.h assembler.h symbol_table.h code_image.h
	$(CC) $(CFLAGS) -c test_first_pass_instructions.c

test_second_pass.o: test_second_pass.c second_pass.h first_pass.h assembler.h external_usage.h
	$(CC) $(CFLAGS) -c test_second_pass.c

test_output_files.o: test_output_files.c output_files.h first_pass.h second_pass.h assembler.h
	$(CC) $(CFLAGS) -c test_output_files.c

test: $(TEST_TARGET) $(TEST_PHASE2_TARGET) $(TEST_PARSER_TARGET) $(TEST_FIRST_PASS_TARGET) $(TEST_FIRST_PASS_INSTRUCTIONS_TARGET) $(TEST_SECOND_PASS_TARGET) $(TEST_OUTPUT_TARGET)
	./$(TEST_TARGET)
	./$(TEST_PHASE2_TARGET)
	./$(TEST_PARSER_TARGET)
	./$(TEST_FIRST_PASS_TARGET)
	./$(TEST_FIRST_PASS_INSTRUCTIONS_TARGET)
	./$(TEST_SECOND_PASS_TARGET)
	./$(TEST_OUTPUT_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_OBJS)

$(TEST_PHASE2_TARGET): $(TEST_PHASE2_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_PHASE2_TARGET) $(TEST_PHASE2_OBJS)

$(TEST_PARSER_TARGET): $(TEST_PARSER_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_PARSER_TARGET) $(TEST_PARSER_OBJS)

$(TEST_FIRST_PASS_TARGET): $(TEST_FIRST_PASS_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_FIRST_PASS_TARGET) $(TEST_FIRST_PASS_OBJS)

$(TEST_FIRST_PASS_INSTRUCTIONS_TARGET): $(TEST_FIRST_PASS_INSTRUCTIONS_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_FIRST_PASS_INSTRUCTIONS_TARGET) $(TEST_FIRST_PASS_INSTRUCTIONS_OBJS)

$(TEST_SECOND_PASS_TARGET): $(TEST_SECOND_PASS_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_SECOND_PASS_TARGET) $(TEST_SECOND_PASS_OBJS)

$(TEST_OUTPUT_TARGET): $(TEST_OUTPUT_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_OUTPUT_TARGET) $(TEST_OUTPUT_OBJS)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(TEST_PHASE2_TARGET) $(TEST_PARSER_TARGET) $(TEST_FIRST_PASS_TARGET) $(TEST_FIRST_PASS_INSTRUCTIONS_TARGET) $(TEST_SECOND_PASS_TARGET) $(TEST_OUTPUT_TARGET) *.o
