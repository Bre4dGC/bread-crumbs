# Contributing to Bread Crumbs

Thank you for your interest in contributing to Bread Crumbs — the programming language for testing, simulations, and AI planning! We encourage contributions to improve features like `solve`, `fork`, and `snapshot`.

## Getting Started

1. Fork the repository.
2. Clone your fork: `git clone https://github.com/your-username/bread-crumbs.git`
3. Create a branch: `git checkout -b your-feature`
4. Make changes and commit: `git commit -m "Add support for tensor in solve"`
5. Push: `git push origin your-feature`
6. Open a Pull Request.

## Installation

- Clone the repo.
- Build: `make`
- Run: `./breadcrumbs your-file.brc`

## Code Style

- Use 4 spaces indentation.
- Follow C-style for the interpreter code.
- For Bread Crumbs scripts: Use snake_case for variables and functions, UpperCase for structures, unions and enums

## Testing

- Add tests for new features using built-in `test` keyword.
- Example:
  ```bread
  test new_feature {
      assert solve x: int where x == 5 == 5
  }
  ```
