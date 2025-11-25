# Bread Crumbs

<div align="center"> 
    <img src="logo.png" alt="Bread Crumbs Logo" width="500"/>
</div>

Bread Crumbs is a modern programming language designed for **testing**, **simulations**, **scientific computing**, **AI planning**, and **rapid prototyping**. With its unique features like declarative problem-solving (`solve`), scenario simulation (`simulate`), state management (`snapshot`/`rollback`), and execution tracing (`trace`), Bread Crumbs empowers developers to tackle complex problems with ease.

## Features

- **Declarative Programming**: Use `solve` to define constraints and let the language find solutions.

  ```brc
  solve (x: int) {
      where x * x == 16
  }
  print(x) // Outputs: 4 or -4
  ```

- **Simulation**: Simulate scenarios with `simulate` and manage state with `snapshot`/`rollback`.

  ```brc
  snapshot state
  simulate scenarios {
      timeline test {
          x += 1
          if (error) { rollback state }
      }
  }
  ```

- **Build-in Testing and Fork the branches**

  ```brc
  test find_numbers {
      solve (x, y: int) {
          where x + y == 10, x * y == 21
      }

      assert(x + y == 10)
      assert(x * y == 21)

      fork scenarios {
          branch positive {
              assert(x > 0 && y > 0)
              print("Positive solution: x = {x}, y = {y}")
          }
          branch negative {
              assert(x < 0 || y < 0)
              print(f"Negative solution: x = {x}, y = {y}")
          }
      }
  }

  run find_numbers
  ```

- **Tracing**: Debug and analyze with `trace` for transparent execution.

## Installation

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/Bre4dGC/bread-crumbs.git
   cd bread-crumbs
   ```

2. **Build from Source**:

   ```bash
   make
   sudo make install
   ```

3. **Verify Installation**:

   ```bash
   crumbs --version
   ```

## Getting Started

Try this simple example to explore Bread Crumbs:

```brc
var numbers: list<int> = [1, 2, 3]
print(numbers) // Outputs: [1, 2, 3]

snapshot state
numbers.push(4)
print(numbers) // Outputs: [1, 2, 3, 4]
rollback state
print(numbers) // Outputs: [1, 2, 3]
```

For more examples, check the [documentation](https://bread-crumbs.org/docs).

## Contributing

We welcome contributions! Here's how to get involved:

1. Fork the repository.
2. Create a branch: `git checkout -b my-feature`.
3. Commit your changes: `git commit -m "Add my feature"`.
4. Push to the branch: `git push origin my-feature`.
5. Open a pull request.

Please read our [Contributing Guidelines](CONTRIBUTING.md) for details.

## Community

Join our community to discuss Bread Crumbs, share ideas, and get help:

- **Discord**: [Join Bread Crumbs Community](https://discord.gg/63pD2dK5u3)
- **GitHub Issues**: Report bugs or suggest features [here](https://github.com/Bre4dGC/bread-crumbs/issues).

## License

Bread Crumbs is licensed under the [MIT License](LICENSE).
