 # Chapter 3: Core C++ API
 
 ## Header Structure
 
 ```cpp
 #include <ipctk.hpp>  // all public API
 ```
 
 The single header provides:
 
 - **IR types** (`ipctk::ir`): `Resource`, `Step`, `Arg`, `Pipe`, `Program`,
   `BackendRule`, `BackendSpec`.
 - **Parser** (`ipctk::parse`): IPC-L and ITKD text → IR.
 - **Validator** (`ipctk::validate`): Semantic correctness checks.
 - **Compiler** (`ipctk::compile`): IR → target code.
 - **Native DSL** (`ipctk::dsl`): C++ expression builders for protocol graphs.
 - **Error handling** (`ipctk::Error`, `ipctk::Result<T>`).
 
 Conditional on Polyflow:
 
 ```cpp
 #if IPCTK_HAS_POLYFLOW
 // graph-processing acceleration enabled
 #endif
 ```
 
 ## IR Types (`ipctk::ir`)
 
 ### Resource
 
 ```cpp
 struct Resource {
   std::string kind{};         // "socket", "shared", "mutex", "queue", "signal", ...
   std::string name{};         // user-assigned identifier
   std::string initializer{};  // e.g., "tcp.listen(\"127.0.0.1:7000\")"
 };
 ```
 
 Resources represent IPC primitives declared at the top of an IPC-L program.
 Each resource has a kind, a name, and an initializer expression.
 
 ### Step and Arg
 
 ```cpp
 struct Arg {
   std::string value{};
   std::optional<std::shared_ptr<Step>> nested{};
 };
 
 struct Step {
   std::string op{};           // e.g., "recv", "decode", "lock", "send"
   std::vector<Arg> args{};    // arguments to the operation
 };
 ```
 
 Steps are the individual operations in a pipe. Arguments can be plain
 strings (resource references or literals) or nested sub-steps.
 
 ### Pipe
 
 ```cpp
 struct Pipe {
   std::string name{};
   std::vector<Step> steps{};
 };
 ```
 
 A pipe is a named sequence of steps representing a dataflow path.
 
 ### Program
 
 ```cpp
 struct Program {
   std::vector<Resource> resources{};
   std::vector<Pipe> pipes{};
   dsl::ASTNode ast{};
 };
 ```
 
 A program is the top-level IR unit: a collection of resources and pipes.
 
 ### BackendRule and BackendSpec
 
 ```cpp
 struct BackendRule {
   std::string operation{}, emit{};
   dsl::ASTNode ast{};
 };
 
 struct BackendSpec {
   std::string target{};
   std::set<std::string> capabilities{};
   std::vector<BackendRule> rules{};
   dsl::ASTNode ast{};
 };
 ```
 
 Backend specs define how operations map to target code. Each rule maps an
 operation name to an emission template.
 
 ## Error Handling
 
 ```cpp
 struct Error {
   enum class Code {
     Parse, Validation, Lowering, Emission, Runtime, Unsupported
   };
   Code code{Code::Runtime};
   std::string message{};
   std::size_t position{0};
 };
 
 template <typename T>
 using Result = dsl::Result<T, Error>;
 ```
 
 All operations that can fail return `Result<T>`. Check with `.is_ok()` /
 `.is_err()`, unwrap with `.unwrap()`, or propagate with early returns.
 
 ```cpp
 auto program = ipctk::parse::parse_program(source);
 if (program.is_err()) {
   std::cerr << "parse error: " << program.unwrap_err().message << "\n";
   return 1;
 }
 auto validated = ipctk::validate::validate(program.unwrap());
 ```
 
 ## Parser API (`ipctk::parse`)
 
 ### Parsing IPC-L Programs
 
 ```cpp
 auto program = ipctk::parse::parse_program(ipcl_source);
 ```
 
 Parses an IPC-L source string into an `ir::Program`. Returns
 `Result<ir::Program>`.
 
 ### Parsing ITKD Backend Specs
 
 ```cpp
 auto backend = ipctk::parse::parse_backend_file("dest/C.itkd");
 ```
 
 Parses a `.itkd` file into an `ir::BackendSpec`. Returns
 `Result<ir::BackendSpec>`.
 
 ### Low-Level Parsers
 
 For advanced use, the parser module exposes composable parsers:
 
 - `parse::ws()` — whitespace skipper.
 - `parse::ident()` — C-style identifier.
 - `parse::token(char)` — single-character token.
 - `parse::until(char)` — balanced-delimiter content.
 - `parse::strip_comments(src)` — strip `//` and `/* */` comments.
 - `parse::trim(sv)` — trim leading/trailing whitespace.
 
 ## Validator API (`ipctk::validate`)
 
 ### Program Validation
 
 ```cpp
 auto validated = ipctk::validate::validate(program);
 ```
 
 Checks:
 
 - Declaration-before-use for all resources.
 - Pipe names are unique and non-empty.
 - Each pipe has at least one step.
 - Step semantics are valid (no circular references, valid resource bindings).
 
 ### Backend Validation
 
 ```cpp
 auto validated = ipctk::validate::validate(backend_spec);
 ```
 
 Checks:
 
 - Target name is non-empty.
 - Each rule has a non-empty operation and emit field.
 - No duplicate operation rules.
 
 ### Compatibility Validation
 
 ```cpp
 auto compatible = ipctk::validate::validate_compatibility(program, backend_spec);
 ```
 
 Checks that every operation in the program has a corresponding rule in the
 backend, that required capabilities are present, and that resource kinds
 match their operations.
 
 ## Compiler API (`ipctk::compile`)
 
 ```cpp
 auto result = ipctk::compile(program, backend_spec);
 ```
 
 The compiler:
 
 1. Validates the program and backend.
 2. Checks compatibility between them.
 3. Lowers the program to per-pipe emission sequences.
 4. Emits target code as a string.
 
 The output string includes:
 
 - Target metadata (`# target <name>`).
 - Capability declarations (`# capability <name>`).
 - Polyflow scheduling status.
 - Per-pipe emitted code.
 
 ## Ownership and Lifetime
 
 - The IR types use value semantics with `std::string` and `std::vector`.
   No manual memory management is required.
 - `Step::args` may contain `std::shared_ptr<Step>` for nested steps;
   these are reference-counted automatically.
 - The `Program` and `BackendSpec` types are self-contained and can be
   freely copied or moved.
 
 ## Concurrency
 
 - The parser, validator, and compiler are reentrant but not thread-safe
   for shared mutable state. Each thread should operate on its own copies.
 - Pipeline composition should isolate synchronization boundaries.
   Shared-state mutation requires explicit lock/unlock stage pairing in
   the IPC-L graph.
