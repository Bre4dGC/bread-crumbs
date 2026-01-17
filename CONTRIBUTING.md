# Contributing to Bread Crumbs

Thank you for your interest in contributing to **Bread Crumbs**, a programming language designed for **testing**, **simulations**, **scientific computing**, **AI planning**, and **rapid prototyping**. Your contributions help us build a powerful and versatile language with unique features like `solve`, `simulate`, `fork`, and `trace`.

## How to Contribute

We welcome contributions in various forms:
- **Code**: Add new features (e.g., support for `tensor` in `solve`), fix bugs, or optimize the compiler.
- **Documentation**: Improve tutorials, README, or API docs.
- **Testing**: Write test cases using `test` and `assert` to ensure stability.
- **Bug Reports**: Report issues via GitHub Issues.
- **Feature Requests**: Suggest ideas to enhance `solve`, `simulate`, or other features.

Before contributing, please read our README to understand the project's goals and features.

## Getting Started

1. **Fork the Repository**:
   - Click "Fork" on the Bread Crumbs repository.
2. **Clone Your Fork**:
```bash
   git clone https://github.com/your-username/bread-crumbs.git
   cd bread-crumbs
```
3. **Create a Branch**:
```bash
   git checkout -b feature/your-feature-name
```
   Use descriptive names, e.g., `feature/tensor-support` or `bugfix/parser-crash`.
4. **Make Changes and Commit**:
```bash
   git commit -m "Add tensor support for solve feature"
```
5. **Push to Your Fork**:
   
```bash
   git push origin feature/your-feature-name
```

6. **Open a Pull Request**:
   - Go to the original repository and create a Pull Request (PR).
   - Provide a clear description of your changes and reference any related issues.

## Installation

To set up the development environment:
1. Clone the repository:
```bash
   git clone https://github.com/bread-crumbs/bread-crumbs.git
   cd bread-crumbs
```

2. Build the compiler:
```bash
   make
```

3. Run a Bread Crumbs file:
```bash
   ./breadcrumbs example.brc
```

# Code Style
To ensure consistency, follow these guidelines for C code in the Bread Crumbs compiler:
- Indentation: Use 4 spaces (no tabs).
- Naming: Use snake_case for variables, functions, and identifiers.
- Function Pointers:
 
```c
datatype* variable_name;
```

- Structures, Unions, and Enums:
 
```c
keyword name {
    // Fields or members
}
```
- Function Declarations:
 
```c
datatype function_name(datatype *parameter_name)
{
    // Control flow
    keyword(...){
        // Statements
    }
}
```

- **Avoid typedef for statements**: Use keyword (e.g., struct, enum) directlyExamplele**:
 
```c
int handle_number(struct token *tok)
{
    if(tok->category == CATEGORY_LITERAL && tok->type_literal == LIT_NUMBER){
        return tok->value;
    }
    return 0;
}
```
For Bread Crumbs code (.brc files):
- Use 4 spaces indentation.
- Use snake_case for function and variable names.
- Example:
 
```brc
func add_numbers(var x: int32, var y: int32) -> int32 {
    return x + y
}

test check_add {
    assert add_numbers(3, 4) == 7
}
```
  
## Testing

All code contributions must include tests to ensure stability. Use the test and assert constructs in Bread Crumbs:
```brc
test solve_example {
    solve x: int where x * x == 16
    assert x == 4 || x == -4
}
```
Run tests with:
```bash
make test
```
## Code Review Process

1. Submit your Pull Request with a clear description.
2. A maintainer will review your code within 3-5 days.
3. Address any feedback and update your PR.
4. Once approved, your changes will be merged.

Please ensure your code:
- Follows the code style guidelines.
- Includes tests for new features or bug fixes.
- Does not introduce breaking changes without discussion.

## Community and Support

Join our community to discuss ideas, ask questions, or collaborated
Issues: Report bugs or suggest features [here](https://github.com/bread-crumbs/bread-crumbs/issues)

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

Thank you for helping make Bread Crumbs better!