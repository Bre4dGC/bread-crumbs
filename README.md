# Bread-Crumbs
# Bread Crumbs: Language Syntax Demonstration  

**Bread Crumbs** is an interpreted C-based programming language designed for experimentation, testing, simulations, and declarative programming. It supports transactional execution, execution branching, built-in tests, and automatic equation system solving. The syntax is minimalist, intuitive, and tailored for developers who want to quickly prototype, debug, and explore alternative scenarios.  

---  
## Key Features  

- **Transactional Execution**: Support for rollback (`rollback`) and state snapshots for safe experimentation.  
- **Execution Branching**: Creating alternative scenarios (`fork`, `merge`) for hypothesis testing.  
- **Built-in Tests**: Writing tests directly in code with automatic data generation.  
- **Declarative Programming**: Solving equation systems and parameter searches using `solve`.  
- **Debugging & Explanation**: Detailed traces (`trace`) and solution explanations (`explain`).  
- **Smart Garbage Collection**: Regional memory for branches and transactions, minimizing overhead.  
- **External Library Integration**: FFI support for calling functions from C, Python, and other languages.  

---  
## Data Types  

### Basic Types  
```rust  
int8, int16, int32, int64   // Signed integers  
uint8, uint16, uint32, uint64 // Unsigned integers  
float32, float64            // Floating-point numbers  
bool                        // Boolean (true/false)  
string                      // Strings  
void                        // No value  
uni                         // Unicode characters  
tensor                      // Tensors  
```  

### Data Structures  
```rust  
list<T>                     // Dynamic list  
map<K, V>                   // Associative array  
vector<T>                   // Fixed-size vector  
tuple<T1, T2, ..., Tn>      // Tuple  
```  

### Custom Types  
```rust  
type Option<T> = Some(T) | None  // Algebraic type (optional value)  
type Result<T, E> = Ok(T) | Err(E)  // Error-handling type  
```  

---  

## Variable and Constant Declarations  

```rust  
// Variables (type inferred if not specified)  
var x = 42              // int32  
var name: string = "Bread"  

// Constants (immutable values)  
const PI: float64 = 3.14159  
const MAX_USERS: int32 = 1000  

// Lists and dictionaries  
var numbers: list<int32> = [1, 2, 3]  
var settings: map<string, int32> = {"timeout": 30, "retries": 3}  
```  

---  

## Input/Output  

```rust  
// Console output  
print("Hello, World!")  

// Console input  
var input: string  
read(input)   

// File operations  
var content: string  
read("file.txt", content)      // Read file  
write("out.txt", "some data")  // Write to file  
```  

---  

## Conditional Statements  

```rust  
var x = 5  
if x == 5 {  
    print("x is 5")  
} elif x > 5 {  
    print("x is greater than 5")  
} else {  
    print("x is less than 5")  
}  
```  

### Switch-like Construct  
```rust  
var input = "yes"  
match input {  
    case "yes":  
        accept()  
    case "no":  
        decline()  
    default:  
        think()  
}  
```  

---  

## Loops  

### While  
```rust  
var i = 0  
while i < 5 {  
    print("i = ", i)  
    i += 1  
}  
```  

### For  
```rust  
// Classic for  
for i = 0; i < 5; i++ {  
    print("i = ", i)  
}  

// Range iteration  
for i in 0..5 {  
    print("i = ", i)  
}  

// List iteration  
var numbers = [10, 20, 30]  
for n in numbers {  
    print("Number: ", n)  
}  
```  

---  

## Functions  

```rust  
// Simple function  
fn add(x: int32, y: int32) int32{  
    return x + y  
}  

// Function call  
var result = add(2, 3)  // result = 5  

// Function with side effects (e.g., I/O)  
fn log_change(x: int32) @io {  
    print("x changed to ", x)  
}  
```  

---  

## Modules  

```rust  
// Importing a module  
import math  

// Using a module function  
var sum = math.add(1, 3)  
```  

---  

## Error Handling  

```rust  
try {  
    var cfg: string  
    read("config.txt", cfg)  
} catch {  
    print("Error: Failed to read file")  
}  
```  

---  

## Structures  
```rust  
struct Point {  
    x: float64  
    y: float64  
}  

var p = Point { x: 1.0, y: 2.0 }  
print("Point: (", p.x, ", ", p.y, ")")  
```  
---  
Data Processing Modules:  
- JSON, YAML, CSV for parsing and serialization.  
- Regular expressions for string processing.  
Networking Capabilities:  
- HTTP client and server.  
- WebSocket for real-time communication.  
```rust  
import json  
import http  

var data = json.parse('{"name": "Bread", "version": 1.0}')  
print("Name: ", data.name)  // Name: Bread  

async fn fetch_user(id: int32) -> string {  
    var response = await http.get("https://api.example.com/users/", id)  
    return response.body  
}  
```  
---  
## Package Manager  
```  
# Installing a library  
crumbs install math-stats  

# Usage in code  
import math_stats  
var mean = math_stats.mean([1, 2, 3, 4, 5])  // 3.0   
```  
## Compilation Support  
- JIT compilation for performance.  
- AOT compilation for executable generation.  
- Optimized `solve` and `simulate` for large datasets.  
```  
# Interpretation  
crumbs run script.bc  

# Compilation  
bread compile script.bc -o script.exe  
./script.exe  
```  
---  
## Advanced Typing  
- Interfaces/Traits: For defining contracts.  
- Generic Functions: For working with multiple types.  
- Sum Types: Extending `Option<T>` and `Result<T, E>`.  

```rust  
trait Printable {  
    fn print(self) -> void  
}  

struct Point { x: float64, y: float64 }  

impl Printable for Point {  
    fn print(self) {  
        print("Point(", self.x, ", ", self.y, ")")  
    }  
}  

var p = Point { x: 1.0, y: 2.0 }  
p.print()  // Point(1.0, 2.0)  
```  
---  

## Transactions and Rollback (snapshot/rollback)  

```rust  
var x = 0  

// Creating a state snapshot  
snapshot state1 = capture(x)  

// Modifying the variable  
x += 10  
print("x after change: ", x)  // x = 10  

// Rolling back to the snapshot  
rollback state1  
print("x after rollback: ", x)  // x = 0  
```  

### Rollback in Loops  
```rust  
var x = 0  
snapshot loop_start  

while x < 5 {  
    x += 1  
    print("x = ", x)  
}  

rollback loop_start  // Reverting all loop changes  
print("x after rollback: ", x)  // x = 0  
```  

---  

## Execution Branching (fork/merge)  

```rust  
var x = 0  

// Creating an alternative branch  
fork timeline fail_case {  
    x = 0  
    fail("This case failed")  
}  

fork timeline success_case {  
    x = 1  
    print("Success case: x = ", x)  
}  

// Selecting the successful branch  
merge success_case  
print("Final x: ", x)  // x = 1  
```  

### Scenario Simulation  
```rust  
simulate scenarios {  
    timeline A { x = 10 }  
    timeline B { x = 20 }  
    timeline C {  
        solve x: int8 where x < 30  
    }  
}  

choose C where x == 12  
print("Chosen x: ", x)  // x = 12  
```  

---  

## Declarative Programming (solve)  

```rust  
// Finding values satisfying conditions  
solve x: int8, y: int8 {  
    where x + y == 10, x > 0, y > 0  
}  
print("Solution: x = ", x, ", y = ", y)  // e.g., x = 5, y = 5  
```  

### Solution Explanation  
```rust  
solve x: int8 {  
    where x * 2 == 20, x < 15  
}  
explain x  
// Output:  
// Tried: x = 0..14  
// Rejected:  
//   x = 0 → constraint failed: x * 2 == 20  
//   x = 1 → constraint failed: x * 2 == 20  
//   ...  
// Accepted: x = 10  
```  

---  

## Built-in Tests  

```rust  
test add_function {  
    assert add(1, 2) == 3  
    assert add(0, 0) == 0  
}  

test edge_case {  
    solve x: int8 {  
        where x * 3 == 99  
    }  
    assert x == 33  
}  
```  

### Branch Testing  
```rust  
test overflow {  
    fork timeline t1 {  
        var x = 999  
        if x >= 1000 {  
            fail "X went over limit"  
        }  
    }  
    rollback t1  // Rollback if test fails  
}  
```  
---  

## Debugging and Tracing  

```rust  
// Enabling tracing  
trace level=debug  

solve x: int8, y: int8 {  
    where x + y == 10, x > 0, y > 0  
}  
print("Result: x = ", x, ", y = ", y)  

// Trace output  
// [DEBUG] Attempt 1: x = 1, y = 9  
// [DEBUG] Attempt 2: x = 2, y = 8  
// ...  
// [DEBUG] Accepted: x = 5, y = 5  
```  

---  

## Asynchronous Programming  

```rust  
async fn fetch_data(url: string) string {  
    var data: string  
    read(url, data) @io  
    return data  
}  

var result = await fetch_data("https://api.example.com")  
print("Data: ", result)  
```  

---  

## External Library Integration (FFI)  

```rust  
extern "libm.so" fn sin(var x: float64) float64  

var result = sin(PI / 2)  
print("sin(π/2) = ", result)  // 1.0  
```  

---  

## Optimization and Smart Garbage Collection  

- **Regional Memory**: Each branch (`fork`) or transaction (`snapshot`) uses an isolated memory region. On `merge`, the region integrates; on `rollback`, it is discarded.  
- **Automatic Garbage Collection**: A combination of reference counting and regional management ensures predictable performance without pauses.  

---  

## Example Complex Program  

```rust  
// Structure definition  
struct Config {  
    timeout: int32  
    retries: int32  
}  

// Function with side effects  
fn load_config(path: string) Config @io {  
    var data: string  
    try {  
        read(path, data)  
        return Config { timeout: 30, retries: 3 }  
    } catch {  
        print("Error loading config")  
        return Config { timeout: 10, retries: 1 }  
    }  
}  

// Scenario testing  
simulate scenarios {  
    timeline default {  
        var cfg = load_config("config.txt")  
        print("Default config: timeout = ", cfg.timeout)  
    }  
    timeline fallback {  
        var cfg = Config { timeout: 5, retries: 2 }  
        print("Fallback config: timeout = ", cfg.timeout)  
    }  
}  

choose default where cfg.timeout > 10  
print("Chosen config: timeout = ", cfg.timeout)  

// Function testing  
test config_load {  
    var cfg = load_config("test.txt")  
    assert cfg.timeout == 30  
    assert cfg.retries == 3  
}  

// Declarative solving  
solve x: int8, y: int8 {  
    where x + y == 10, x > 2, y > 2  
}  
trace level=debug  
explain x, y  
print("Solution: x = ", x, ", y = ", y)  
``` 
