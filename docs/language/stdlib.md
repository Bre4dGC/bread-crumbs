# Standard Library

The standard library is currently limited to the builtin console I/O functions
provided by the runtime. Additional modules (`std.fs`, `std.net`, `std.console`,
...) are still in the design stage.

## Builtins

These names are available in every program without an `import`:

| Name          | Signature              | Description                                  |
|---------------|------------------------|----------------------------------------------|
| `print`       | `print(any) -> void`   | Write a value to stdout, followed by `\n`.   |
| `write`       | `write(any) -> void`   | Write a value to stdout, without `\n`.       |
| `eprint`      | `eprint(any) -> void`  | Write a value to stderr, followed by `\n`.   |
| `read_line`   | `read_line() -> str`   | Read one line from stdin (newline stripped). |
| `read_all`    | `read_all() -> str`    | Read all of stdin.                           |

The runtime picks the correct C function based on the static type of the
argument:

- `str` / `char` / `bool` / `int` / `float` / `decimal`

## Example

```brc
func main() {
    print("Hello, World!")
    print(42)
    print(true)
    write("no newline")
    write("...")
}
```

Output:

```
Hello, World!
42
true
no newline...
```

## Memory

Strings returned by `read_line()` and `read_all()` are allocated by the runtime.
They are currently never freed automatically; call `brt_free` from C or wait for
the memory management layer to be implemented.
