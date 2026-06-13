# Lexer - Documentation

This document provides an overview of the lexer component of the compiler, which is responsible for tokenizing the source code into a stream of tokens that can be consumed by the parser.

---

## Tokens

Tokens are the basic building blocks of the source code, representing keywords, operators, delimiters, identifiers, and literals. Each token has a type and a literal value.

```c
enum category_service   {...}
enum category_operator  {...}
enum category_keyword   {...}
enum category_paren     {...}
enum category_delimiter {...}
enum category_datatype  {...}
enum category_literal   {...}
enum category_modifier  {...}

enum category_tag {
    CAT_SERVICE,   CAT_OPERATOR,
    CAT_KEYWORD,   CAT_PAREN,
    CAT_DELIMITER, CAT_DATATYPE,
    CAT_LITERAL,   CAT_MODIFIER,
}

struct token_t {
	const char* literal;        // "if", "+", "42", etc.
    int type;                   // KW_IF, OPER_PLUS, LITER_NUMBER, etc.
    enum category_tag category; // CAT_KEYWORD, CAT_OPERATOR, etc.
}

void init_tokens(void); // Initializes the token system, setting up hashmap
token_t new_token(const enum category_tag category, const int type, const char* literal); // Creates a new token with the specified category, type, and literal value.
token_t* find_token(const char* potential); // Searches for an existing token in hashmap with the given literal value.
void free_tokens(void); // Frees the hashmap that was created at initialization
```

---

### Keywords

#### Control flow

`if` — Conditional branch. Single-statements must be wrapped in a block.
```
if(x < 10) {...}
```

`else` — Fallback branch of an if statement.
```
if(...) {...} else {...}
```

`elif` — Else-if branch in a conditional chain.
```
if(...) {...} elif(x < 20) {...} else {...}
```

`do` — Declares a do-while loop, which executes the block at least once before checking the condition.
```
do {...} while(x < 10)
```

`while` — Declares a while loop, which executes the block repeatedly as long as the condition is true.
```
while(x < 10) {...}
```

`for` — Declares a for loop, which consists of an initializer, a condition, and an increment expression. It can also be used for iterating over collections with the `..` range operator.
```
for(var i; i < 10; i++) {...}
for(item = ..collection.elems) {...}
```

`func` — Declares a function with an optional return type.
```
func my_function() : int {...}
```

`static` — Declares a static variable or function, which is shared across all instances of a type or is limited to the current translation unit.
```
static var count : int = 0
```

`return` — Exits the current function and optionally returns a value to the caller.
```
return 42
```

`break` — Exits the nearest enclosing loop or switch statement
```
while(true) { break }
```

`continue` — Skips to the next iteration of the nearest enclosing loop.
```
for(var i; i < 10; i++) { if(i == 5) { continue } };
```

`default` — Default case in a switch statement
```
match(x) { ... default {...} }
```

`match` — Multi-branch conditional on a value.
```
match(x) { case(1) {...} ... default {...} }
```

`case` — Defines a branch in a switch statement
```
match(x) { case (1) {...} case(2) {...} case(..) {...} }
```

`struct` — Declares a composite data type with named fields.
```
struct { var x : int; var y : int }
```

`enum` — Declares an enumeration of named integer constants.
```
enum { Red, Green, Blue }
```

`import` — Imports a module or specific items from a module.
```
import path.to.module
```

`module` — Declares the current file as a module with a specified name.
```
module my.module.name
```

`type` — Declares a type alias or a union type.
```
type my_type = int | str
```

`self` — Reference to the current instance in a method
```
func my_struct.my_method() {self.x = 10}
```

`trait` — Declares a contract that an object must implement.
```
trait my_trait { func my_method() : int }
```

`impl` — Implementation of a trait for a type
```
impl my_trait for my_struct { func my_method() : int {...} }
```

`try` — Starts a block of code that will be tested for exceptions.
```
try failing_function()
```

`catch` — Handles a thrown exception.
```
failing_function() catch (e) {...}
```

`throw` — Throws an exception with a specified value.
```
throw "Something went wrong"
```

#### Types

`int`     — Integer type
`uint`    — Unsigned integer type
`short`   — Short integer type
`ushort`  — Short unsigned integer type
`long`    — Long integer type
`ulong`   — Long unsigned integer type
`char`    — Character type
`byte`    — Byte type
`float`   — Floating-point type
`decimal` — Decimal type
`str`     — String type
`bool`    — Boolean type
`any`     — Any type

#### Modifiers

`var`    — Declares a mutable variable.
`const`  — Declares an immutable variable that must be initialized at the time of declaration.
`final`  — Declares a variable that can only be assigned once, but does not require initialization at the time of declaration.
`ref`    — Declares a reference to an existing variable, allowing for aliasing and shared mutability.

#### Literals

`null`         — Null value, representing the absence of a value or a non-existent reference.
`true` `false` — Boolean literals representing the two possible values of the boolean type.
`infinity`     — Represents positive infinity, a special value that is greater than all finite numbers.

---

### Operators

#### Arithmetic

`+`  — Addition

`-`  — Subtraction

`*`  — Multiplication

`/`  — Division

`%`  — Modulo

#### Assignment

`=`  — Assignment

`+=` — Addition assignment

`-=` — Subtraction assignment

`*=` — Multiplication assignment

`/=` — Division assignment

`%=` — Modulo assignment

#### Comparison

`==` — Equality

`!=` — Inequality

`<`  — Less than

`>`  — Greater than

`<=` — Less than or equal to

`>=` — Greater than or equal to

#### Logical

`&&` — AND

`||` — OR

`!`  — NOT

`|`  — Alternative OR

`&`  — Alternative AND

#### Other

`++` — Increment

`--` — Decrement

`.`  — Member access, used for accessing properties or methods of an object or struct.

`,`  — Comma, used for separating items in lists, function arguments, etc.

`:`  — Type annotation or label

`?`  — Optional chaining, used to safely access properties or call functions on potentially null or undefined values.

`..` — Range operator, used for creating ranges of values.

### Delimiters

`(`  — Left parenthesis

`)`  — Right parenthesis

`{`  — Left curly brace

`}`  — Right curly brace

`[`  — Left square bracket

`]`  — Right square bracket

`;`  — Semicolon, used to terminate statements.

---

### Identifiers

`a-zA-Z0-9_` — Valid characters for identifiers (variable names, function names, etc.)

---

### Literals

#### Number value

`0-9`          — Integer

`0-9.0-9`      — Floating-point number

`0x0-9a-fA-F`  — Hexadecimal

`0o0-7`        — Octal

`0b0-1`        — Binary

#### String value

`'A'`     — Character

`"Hello"` — String

---

## Lexer

```cpp
struct lexer_t {
    char ch;        // Current character being processed in the input stream.
    location_t loc; // Current location in the source code, typically including line and column information for error reporting and debugging purposes.
    size_t balance; // Used to track the balance of parentheses, braces, and brackets to ensure proper nesting and scope management during tokenization.
    compiler_context_t* ctx; // Pointer to the compiler context, which may contain information about the source code, error handling, and other relevant data needed during the lexing process.
}

lexer_t* new_lexer(compiler_context_t* ctx); // Creates a new lexer instance with the given compiler context.
token_t next_token(lexer_t* lexer); // Retrieves the next token from the input stream, advancing the lexer's position accordingly.
```

## Tokenization

```cpp
// Initial size for buffers, will be dynamically resized if needed
#define IDENT_SIZE 16
#define NUM_SIZE 32
#define STR_SIZE 64

// Maximum allowed size for identifiers, numbers, and strings to prevent excessive memory usage
#define MAX_IDENT_SIZE 64
#define MAX_NUM_SIZE 128
#define MAX_STR_SIZE 4096

// Token handling functions for different categories
token_t handle_operator(lexer_t* lexer);
token_t handle_paren(lexer_t* lexer);
token_t handle_number(lexer_t* lexer);
token_t handle_ident(lexer_t* lexer);
token_t handle_string(lexer_t* lexer);
void handle_comment(lexer_t* lexer);

// Functions to read identifiers, numbers, and strings from the input stream
string_t read_ident(lexer_t* lexer);
string_t read_number(lexer_t* lexer, enum category_literal* lit);
string_t read_string(lexer_t* lexer, char quote_char);
char read_escseq(lexer_t* lexer);

// Utility functions for character handling
void read_ch(lexer_t* lexer); // Advances the lexer to the next character in the input stream, updating the current character and location.

void skip_whitespace(lexer_t* lexer); // Skips over any whitespace characters in the input stream, advancing the lexer's position until a non-whitespace character is encountered.

char peek_ch(const lexer_t* lexer); // Returns the next character in the input stream without advancing the lexer's position, allowing for lookahead functionality during tokenization.
```