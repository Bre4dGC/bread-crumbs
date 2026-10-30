# Language Token References

## Control flow

`if` — Conditional branch. Single-statements must be wrapped in a block.
```
if(x < 10){...}
```

`else` — Fallback branch of an if statement.
```
if(...){...} else {...}
```

`elif` — Else-if branch in a conditional chain.
```
if(...){...} elif(x < 20){...} else {...}
```

`while` — Declares a while loop, which executes the block repeatedly as long as the condition is true.
```
while(x < 10){...}
```

`for` — Declares a for loop, which consists of an initializer, a condition, and an increment expression. It can also be used for iterating over collections with the `..` range operator.
```
for(var i; i < 10; i++){...}
for(item = ..collection.elems){...}
```

`func` — Declares a function with an optional return type.
```
func my_function() : int {...}
```

`return` — Exits the current function and optionally returns a value to the caller.
```
return 42
```

`break` — Exits the nearest enclosing loop or switch statement
```
while(true){ break }
```

`continue` — Skips to the next iteration of the nearest enclosing loop.
```
for(var i; i < 10; i++){ if(i == 5){ continue } };
```

`default` — Default case in a switch statement
```
match(x){ ... default {...} }
```

`match` — Multi-branch conditional on a value.
```
match(x){ case(1){...} ... default {...} }
```

`case` — Defines a branch in a switch statement
```
match(x){ case (1){...} case(2){...} case(..){...} }
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

### Types

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

### Modifiers

`var`    — Declares a mutable variable.
`const`  — Declares an immutable variable that must be initialized at the time of declaration.
`final`  — Declares a variable that can only be assigned once, but does not require initialization at the time of declaration.

### Literals

`null`         — Null value, representing the absence of a value or a non-existent reference.
`true` `false` — Boolean literals representing the two possible values of the boolean type.
`self`         —

---

## Operators

### Arithmetic

`+`  — Addition

`-`  — Subtraction

`*`  — Multiplication

`/`  — Division

`%`  — Modulo

### Assignment

`=`  — Assignment

`+=` — Addition assignment

`-=` — Subtraction assignment

`*=` — Multiplication assignment

`/=` — Division assignment

`%=` — Modulo assignment

### Comparison

`==` — Equality

`!=` — Inequality

`<`  — Less than

`>`  — Greater than

`<=` — Less than or equal to

`>=` — Greater than or equal to

### Logical

`&&` — AND

`||` — OR

`!`  — NOT

`|`  — Alternative OR

`&`  — Alternative AND

### Other

`++` — Increment

`--` — Decrement

`.`  — Member access, used for accessing properties or methods of an object or struct.

`,`  — Comma, used for separating items in lists, function arguments, etc.

`:`  — Type annotation or label

`?`  — Optional chaining, used to safely access properties or call functions on potentially null or undefined values.

`..` — Range operator, used for creating ranges of values.

## Delimiters

`(`  — Left parenthesis

`)`  — Right parenthesis

`{`  — Left curly brace

`}`  — Right curly brace

`[`  — Left square bracket

`]`  — Right square bracket

`;`  — Semicolon, used to terminate statements.

---

## Identifiers

`a-zA-Z0-9_` — Valid characters for identifiers (variable names, function names, etc.)

---

## Literals

### Number value

`0-9`          — Integer

`0-9.0-9`      — Floating-point number

`0x0-9a-fA-F`  — Hexadecimal

`0o0-7`        — Octal

### String value

`'A'`     — Character

`"Hello"` — String
