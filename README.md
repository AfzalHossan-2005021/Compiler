# C-like Compiler Front End and 8086 Code Generator

This repository contains a manual implementation of a compiler pipeline for a course-defined C-like language subset. It includes a scoped symbol table, Flex-based lexical analysis, Bison-based syntax and semantic analysis, parse-tree/AST-style representation, 8086-style intermediate code generation, and peephole optimization.

The project was developed from formal compiler-sessional specifications at BUET and focuses on explicit implementation of compiler mechanics rather than relying on high-level compiler frameworks.

## Core Capabilities

- Scoped symbol-table implementation with hash tables, chaining, nested scopes, and parent-scope lookup
- Lexical analyzer for keywords, identifiers, literals, operators, punctuators, comments, and lexical errors
- Bison parser for a C-like grammar with declarations, functions, statements, expressions, arrays, and control flow
- Semantic analyzer for declaration checks, type consistency, array usage, function signatures, and parameter validation
- Parse tree generation with grammar-rule logging and line-aware diagnostics
- 8086-style assembly generation for expressions, assignments, control flow, functions, arrays, and `println`
- Stack-based local variable and function-parameter handling
- Peephole optimization for selected redundant assembly instruction patterns

## Compiler Pipeline

| Stage | Directory | Implementation | Primary Output |
| --- | --- | --- | --- |
| Symbol table | `SymbolTable/` | C++ | Scoped identifier storage and lookup |
| Lexical analysis | `LexicalAnalyzer/` | Flex, C++ | Token stream, logs, lexical diagnostics |
| Syntax analysis | `SyntaxSemanticAnalyzer/` | Flex, Bison, C++ | Parse tree and grammar-rule logs |
| Semantic analysis | `SyntaxSemanticAnalyzer/` | Bison semantic actions, C++ | Type/scope/function diagnostics |
| Code generation | `IntermediateCodeGenerator/` | Bison, C++ tree traversal | `code.asm` |
| Optimization | `IntermediateCodeGenerator/` | C++ peephole pass | `optimized_code.asm` |

## Specification Mapping

The implementation follows four staged specifications:

### 1. Symbol Table

- `SymbolInfo`, `ScopeTable`, and `SymbolTable` abstractions
- sdbm-style hashing with separate chaining
- nested scope creation and removal
- current-scope and parent-scope lookup
- insert, lookup, delete, print-current, and print-all operations
- file-driven command processing through `input.txt`

### 2. Lexical Analysis

- token recognition using Flex
- keywords, identifiers, constants, operators, punctuators, strings, and characters
- single-line and multi-line comment handling
- line tracking and diagnostic logging
- symbol-table interaction for identifiers where required by the phase

### 3. Syntax and Semantic Analysis

- parser generated from Bison grammar rules
- conflict handling for ambiguous constructs such as `if`/`else`
- parse-tree construction and matched-rule logging
- scoped declaration insertion
- semantic validation for:
  - undeclared identifiers
  - duplicate declarations in the same scope
  - assignment type compatibility
  - array indexing and array/non-array misuse
  - modulus operand types
  - function declaration/definition consistency
  - function-call argument count and type matching
  - invalid use of `void` expressions

### 4. Intermediate Code Generation

- 8086-style assembly as the intermediate representation
- tree traversal after successful syntax and semantic analysis
- stack-frame setup for functions
- stack-based local variables and function parameters
- register-based return values
- labels and conditional jumps for control flow
- short-circuit style boolean code generation
- generated `println` procedure
- source-line comments in emitted assembly
- peephole optimization for redundant `MOV`, redundant `PUSH`/`POP`, and no-op arithmetic patterns

## Supported Language Subset

The compiler targets a BUET CSE 310 C-like language subset, including:

- types: `int`, `float`, `void`
- global and local declarations
- one-dimensional arrays
- function declarations and definitions
- parameterized function calls
- assignment, arithmetic, relational, logical, unary, increment, and decrement expressions
- `if`, `if-else`, `for`, and `while`
- `return`
- `println`

It is not intended to be a complete ISO C compiler.

## Grammar Coverage

The parser implementation covers the main grammar families from the supplied Bison grammar:

- program structure: `start`, `program`, `unit`
- declarations: `var_declaration`, `declaration_list`, `type_specifier`
- functions: `func_declaration`, `func_definition`, `parameter_list`, `argument_list`, `arguments`
- scopes and statements: `compound_statement`, `statements`, `statement`
- expressions: `expression`, `logic_expression`, `rel_expression`, `simple_expression`, `term`, `unary_expression`, `factor`
- variables and arrays: `variable`

This grammar supports operator-precedence-aware expression parsing, nested compound statements, declarations inside functions, array indexing, expression statements, control-flow statements, and function calls.

## Repository Structure

```text
Compiler/
|-- SymbolTable/
|   |-- 2005021_Main.cpp
|   |-- 2005021_SymbolTable.h
|   |-- input.txt
|   `-- output.txt
|
|-- LexicalAnalyzer/
|   |-- 2005021.l
|   |-- 2005021.h
|   `-- input.txt
|
|-- SyntaxSemanticAnalyzer/
|   |-- 2005021.l
|   |-- 2005021.y
|   |-- 2005021.h
|   |-- Makefile
|   `-- input.c
|
|-- IntermediateCodeGenerator/
|   |-- code/
|   |   |-- 2005021.l
|   |   |-- 2005021.y
|   |   |-- ast_utils.h
|   |   |-- lex_utils.h
|   |   `-- Makefile
|   `-- input/
|       |-- test1_i.c
|       |-- test2_i.c
|       |-- test3_i.c
|       |-- test4_i.c
|       |-- test5_i.c
|       |-- test6_i.c
|       |-- test7_i.c
|       |-- exp.c
|       |-- func.c
|       `-- loop.c
|
|-- README.md
`-- LICENSE
```

## Requirements

- `g++`
- `gcc`
- `flex`
- `bison`
- `make`

Ubuntu/Debian:

```bash
sudo apt-get install flex bison build-essential make
```

Windows users should use WSL2, MSYS2, or another Unix-like environment with Flex, Bison, `g++`, and `make`.

## Running the Components

### Symbol Table

```bash
cd SymbolTable
g++ 2005021_Main.cpp -o symbol_table
./symbol_table
```

Reads commands from `input.txt` and writes results to `output.txt`.

### Lexical Analyzer

```bash
cd LexicalAnalyzer
flex 2005021.l
gcc lex.yy.c -lfl -o lexer
./lexer input.txt
```

If your environment expects standard input:

```bash
./lexer < input.txt
```

### Syntax and Semantic Analyzer

```bash
cd SyntaxSemanticAnalyzer
make
```

The default target builds the lexer/parser and runs the analyzer on `input.c`.

Expected generated files:

- `log.txt`
- `error.txt`
- `parse_tree.txt`

### Intermediate Code Generator

```bash
cd IntermediateCodeGenerator/code
make test1
```

Available test targets:

```bash
make test2
make test3
make test4
make test5
make test6
make test7
make exp
make func
make loop
make bonustest1
make bonustest2
```

Expected generated files:

- `code.asm`
- `optimized_code.asm`
- `log.txt`
- `error.txt`
- `parse_tree.txt`

## Example Source Program

```c
int i, j;

int main() {
    int k;

    i = 1;
    println(i);

    j = 5 + 8;
    println(j);

    k = i + 2 * j;
    println(k);

    return 0;
}
```

For valid input programs, the final phase emits 8086-style assembly with stack-frame setup, expression evaluation, labels, conditional jumps, function-return handling, and calls to the generated `println` routine.

## Implementation Notes

- The symbol table is implemented manually using dynamically allocated chained hash tables.
- Each scope owns a separate `ScopeTable` and links to its parent scope.
- Parser semantic actions construct tree nodes, update symbol metadata, and report semantic errors with source-line context.
- Code generation traverses the tree representation and emits assembly incrementally.
- Local variables are addressed through stack offsets rather than data-segment declarations.
- Boolean expressions use jump-oriented code generation where appropriate.
- The optimizer performs a focused peephole pass after assembly generation.

## Limitations

- The language is a course-defined subset of C, not full C.
- Floating-point code generation is intentionally limited by the intermediate-code-generation specification.
- Error recovery is limited to the implemented grammar actions.
- Generated build artifacts such as `lex.yy.c`, `*.tab.c`, `*.tab.h`, `a.out`, logs, and assembly files are excluded from the source-focused layout.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
