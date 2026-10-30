# Runtime

Bread Crumbs programs are linked against a small C runtime library that
provides functionality the language itself does not express, such as console
I/O.

## Artifact

The runtime is built as a static archive:

```
lib/libbrc.a
```

It is produced by `make` (or the `breadcrumbs_rt` CMake target) and installed
to `/usr/local/lib` by `make install`.

## Linking

`crum build` compiles a source file to an executable and links the runtime
automatically. The compiler looks for the archive in this order:

1. `CRUM_RUNTIME_LIB` (full path to the archive)
2. The build directory recorded when the compiler was built (`lib/`)
3. `./lib/libbrc.a`
4. `/usr/local/lib/libbrc.a`

Use `crum build <source> -c` to emit an object file without linking.

## ABI

| Language type | C type        | LLVM type |
|---------------|---------------|-----------|
| `str`         | `const char*` | `i8*`     |
| `int`         | `int32_t`     | `i32`     |
| `float`       | `double`      | `double`  |
| `decimal`     | `double`      | `double`  |
| `bool`        | `bool`        | `i1`      |
| `char`        | `char`        | `i8`      |

Strings are UTF-8 and NUL terminated. Strings returned by the runtime are
allocated with `malloc` and must be released with `brt_free`.

## Console I/O

The public header is [`include/runtime/io.h`](../include/runtime/io.h).

### Output

```c
void brt_print(const char* s);        /* stdout, newline */
void brt_print_int(int32_t v);
void brt_print_float(double v);
void brt_print_bool(bool v);
void brt_print_char(char c);

void brt_write(const char* s);        /* stdout, no newline */
void brt_write_int(int32_t v);
void brt_write_float(double v);
void brt_write_bool(bool v);
void brt_write_char(char c);

void brt_eprint(const char* s);       /* stderr, newline */
void brt_eprint_int(int32_t v);
void brt_eprint_float(double v);
void brt_eprint_bool(bool v);
void brt_eprint_char(char c);

void brt_flush(void);
```

### Input

```c
char* brt_read_line(void);  /* NULL on EOF */
char* brt_read_all(void);
void  brt_free(void* p);
```

## Language builtins

The compiler maps the following builtins onto the runtime:

| Builtin       | Behaviour                                   |
|---------------|---------------------------------------------|
| `print(x)`    | Write `x` to stdout followed by a newline.  |
| `write(x)`    | Write `x` to stdout without a newline.      |
| `eprint(x)`   | Write `x` to stderr followed by a newline.  |
| `read_line()` | Read one line from stdin and return a `str`.|
| `read_all()`  | Read all of stdin and return a `str`.       |

```brc
func main() {
    print("What is your name?")
    var name : str = read_line()
    print("Hello, " + name)
}
```

> Note: expression concatenation is not implemented yet; the example above is
> illustrative. `print(name)` works today.
