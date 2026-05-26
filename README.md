# 🔧 Compiler — Mini C Compiler Suite

> **A production-ready compiler implementation for a C-like language subset, featuring full lexical analysis, syntax & semantic parsing, a scoped symbol table, and intermediate code generation with optimization.**

<div align="center">

![Build Status](https://img.shields.io/badge/status-active-success.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B%20%7C%20C-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

</div>

---

## 🎯 Overview

This is a **comprehensive compiler project** demonstrating all major phases of compiler design and implementation. Built with **Flex** and **Bison**, it provides a complete pipeline from source code tokenization through intermediate code generation with optimization.

### What Makes This Special

✨ **Production-grade architecture** | 🔍 **Full error reporting** | 📊 **AST generation** | ⚡ **Code optimization** | 🎓 **Educational value**

---

## ✨ Core Compiler Phases

| Phase | Component | Technology | Features |
|-------|-----------|-----------|----------|
| **Lexical Analysis** | LexicalAnalyzer | Flex | Token generation, line tracking |
| **Syntax Analysis** | SyntaxSemanticAnalyzer | Bison | AST construction, parse tree output |
| **Semantic Analysis** | SyntaxSemanticAnalyzer | Bison + C++ | Type checking, scope validation, error reporting |
| **Symbol Management** | SymbolTable | C++ | Scoped hash table, redefinition detection |
| **Code Generation** | IntermediateCodeGenerator | Bison + C++ | Assembly-like IR generation |
| **Optimization** | IntermediateCodeGenerator | C++ | Code simplification & peephole optimization |

---

## 🚀 Key Features

### Language Support
- **Data Types**: `int`, `float`, `void`
- **Functions**: Declaration, definition, parameter passing, return types
- **Control Flow**: `if`/`else`, `for`, `while` loops
- **Data Structures**: Variables, arrays, expressions
- **Type System**: Full type checking and type coercion

### Compiler Capabilities
- ✅ **Comprehensive Error Detection** — Semantic errors with line numbers and descriptions
- ✅ **AST Generation** — Complete abstract syntax tree representation
- ✅ **Parse Tree Output** — Text-based visualization of derivations
- ✅ **Scoped Symbol Table** — Function and block-level scope management
- ✅ **Redefinition Checking** — Prevents duplicate function/variable declarations
- ✅ **Intermediate Code** — Assembly-like representation (3-address code)
- ✅ **Code Optimization** — Simple but effective optimizer reducing code size
- ✅ **Detailed Logging** — Parse progress, errors, and optimization steps

---

## 📁 Project Architecture

```
Compiler/
├── LexicalAnalyzer/              # Phase 1: Tokenization
│   ├── 2005021.l                 # Flex lexer specification
│   └── input.txt                 # Sample lexer input
│
├── SyntexSemanticAnalyzer/       # Phase 2 & 3: Parsing & Semantics
│   ├── 2005021.y                 # Bison grammar rules & semantic actions
│   ├── 2005021.h                 # Header with AST node definitions
│   ├── 2005021.l                 # Flex lexer for this phase
│   ├── Makefile                  # Build automation
│   └── input.c                   # Sample source code
│
├── IntermediateCodeGenerator/    # Phase 4 & 5: Code Gen & Optimization
│   ├── code/
│   │   ├── 2005021.y             # Enhanced Bison grammar for IR
│   │   ├── 2005021.l             # Flex lexer
│   │   ├── ast_utils.h           # AST node utilities
│   │   ├── lex_utils.h           # Lexer utilities
│   │   └── Makefile              # Build with test targets
│   └── input/                    # Test cases (test1-7, loop, func, exp)
│
├── SymbolTable/                  # Symbol Table Implementation
│   ├── 2005021_SymbolTable.h     # Scoped hash table with chaining
│   ├── 2005021_Main.cpp          # CLI test harness
│   └── input.txt                 # Symbol table test commands
│
├── README.md                      # This file
└── LICENSE                        # Project license
```

---

## 🛠️ Prerequisites

| Requirement | Purpose |
|-------------|---------|
| **flex** | Lexical analyzer generator |
| **bison** | Parser generator |
| **g++** | C++ compiler (C++11 or higher) |
| **gcc** | C compiler |
| **make** | Build automation tool |

### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install flex bison build-essential
```

**macOS (Homebrew):**
```bash
brew install flex bison
```

**Windows:** Use WSL2, MSYS2, or MinGW with these tools installed

---

## ⚡ Quick Start

### 1️⃣ Syntax & Semantic Analysis

```bash
cd SyntexSemanticAnalyzer
make         # Compile: flex + bison + g++
make run     # Execute with input.c
```

**Outputs:**
- `error.txt` — Compilation errors (if any)
- `parse_tree.txt` — Derivation parse tree
- `log.txt` — Compiler logs

### 2️⃣ Intermediate Code Generation

```bash
cd IntermediateCodeGenerator/code
make test1   # Run test case 1
# or
make exp     # Run expression example
```

**Outputs:**
- `code.asm` — Generated assembly-like code
- `optimized_code.asm` — Optimized version

### 3️⃣ Symbol Table Testing

```bash
cd SymbolTable
make         # or: g++ 2005021_Main.cpp -o symbol_table
./symbol_table
```

**Outputs:**
- `output.txt` — Symbol table operations results

### 4️⃣ Lexical Analysis Only

```bash
cd LexicalAnalyzer
flex 2005021.l
gcc lex.yy.c -lfl -o lexer
./lexer < input.txt
```

---

## 📚 Workflow & Usage

### Processing a C-like Source File

1. **Drop your source code** (e.g., `myprogram.c`) into `SyntexSemanticAnalyzer/`
2. **Run the analyzer:**
   ```bash
   cd SyntexSemanticAnalyzer
   make run INPUT=myprogram.c
   ```
3. **Check outputs:**
   - `error.txt` → Any compilation issues
   - `parse_tree.txt` → How your code was parsed
   - `log.txt` → Detailed compilation steps

### Testing Code Generation

The `IntermediateCodeGenerator/code/Makefile` includes test targets:
- `make test1` through `make test7` — Various language features
- `make loop` — Loop constructs
- `make func` — Function definitions
- `make exp` — Expression evaluation
- `make run` — Default test build

---

## 🎓 Educational Value

This project exemplifies:
- ✅ Compiler architecture and design patterns
- ✅ Lexical analysis with finite state machines
- ✅ Context-free grammar and parsing algorithms
- ✅ Abstract syntax tree construction
- ✅ Symbol table design with scoping
- ✅ Semantic analysis and type checking
- ✅ Intermediate code generation
- ✅ Code optimization techniques
- ✅ Error handling and reporting

---

## 📝 Example

**Input C-like code** (`input.c`):
```c
int main() {
    int x = 5;
    float y = 3.14;
    if (x > 0) {
        return x + y;
    }
    return 0;
}
```

**Generated Intermediate Code** (`code.asm`):
```asm
; Assembly-like intermediate representation
; with variables, function calls, jumps, etc.
```

**Optimized Output** (`optimized_code.asm`):
```asm
; Reduced, streamlined version
; with dead code elimination, constant folding, etc.
```

---

## 🔧 Build Details

### Manual Build

**Syntax & Semantic Analyzer:**
```bash
cd SyntexSemanticAnalyzer
bison -d 2005021.y       # Generate parser
flex 2005021.l           # Generate lexer
g++ 2005021.tab.c lex.yy.c -o analyzer
./analyzer input.c
```

**With Make** (simpler):
```bash
make        # Auto-runs all above commands
make run    # Execute with input.c
```

---

## 🧪 Testing Suites

### Intermediate Code Generator Tests
Located in `IntermediateCodeGenerator/input/`:
- `test1_i.c` — Basic variable declarations
- `test2_i.c` — Arithmetic expressions
- `test3_i.c` — Control flow (if/else)
- `test4_i.c` — Loops (for/while)
- `test5_i.c` — Function calls
- `test6_i.c` — Arrays and indexing
- `test7_i.c` — Complex programs
- `loop.c` — Loop optimization test
- `func.c` — Function handling
- `exp.c` — Expression evaluation
- `bonustest1_i.c`, `bonustest2_i.c` — Advanced features

---

## 📊 Code Statistics

| Component | Files | Technology | Purpose |
|-----------|-------|-----------|---------|
| Lexical Analyzer | 2 files | Flex (`.l`) | Tokenization |
| Syntax Analyzer | 3 files | Bison (`.y`) + C++ Header | Parsing & AST |
| Symbol Table | 2 files | C++ (`.h`, `.cpp`) | Scope management |
| Code Generator | 4 files | Bison (`.y`) + C++ Utilities | IR & Optimization |

---

## 🎯 Design Highlights

### Modular Architecture
Each phase is independent and can be tested separately, making the codebase easy to understand and maintain.

### Comprehensive Error Handling
The compiler provides detailed error messages with exact line numbers, making debugging source code straightforward.

### Optimized Output
The intermediate code generator includes an optimizer that performs:
- Dead code elimination
- Constant folding
- Redundant instruction removal
- Code simplification

### Scoped Symbol Table
Properly handles function scope and block scope with a hash table implementation featuring collision resolution through chaining.

---

## 📖 Documentation Output

Each compilation generates detailed documentation:
- **Parse Tree** — Visual representation of grammar derivations
- **Error Log** — All syntax and semantic errors
- **Compiler Log** — Phase-by-phase progress
- **Intermediate Code** — Generated assembly-like code
- **Optimized Code** — Final optimized representation

---

## 💡 Tips & Tricks

- **View generated files** after running — Check `error.txt`, `parse_tree.txt`, `code.asm` for compiler output
- **Run individual test cases** from IntermediateCodeGenerator — Use `make test1`, `make test2`, etc.
- **Symbol table debugging** — Check the `output.txt` file for operation results
- **Lexer testing** — Use `LexicalAnalyzer/input.txt` for quick tokenization tests

---

## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

---

## 👨‍💻 Author

Built as an educational compiler implementation project.

---

<div align="center">

**Made with ❤️ for educational purposes**

[⬆ Back to top](#-compiler--mini-c-compiler-suite)

</div>

Commands in `input.txt` include `I` (Insert), `L` (Lookup), `D` (Delete), `P` (Print), `S` (Enter Scope), `E` (Exit Scope), `Q` (Quit).

---

## 🧪 Test Inputs

Look under `IntermediateCodeGenerator/input/` for sample C-like inputs (e.g. `test1_i.c`, `exp.c`, `func.c`, `loop.c`), and `SyntexSemanticAnalyzer/input.c` for parser tests.

---

## ✅ How to Contribute

- Open an issue describing the improvement or the bug
- Create a branch, add tests or examples, and send a PR
- Follow the existing code style: comments, logging behavior and clear error messages

---

## 📜 License

See the `LICENSE` file in the repository root for licensing details.

---
