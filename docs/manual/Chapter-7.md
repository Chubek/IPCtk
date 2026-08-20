 # Chapter 7: Testing, Fuzzing, and Reliability
 
 IPCtk has a two-tier testing strategy: deterministic unit tests for
 behavioral correctness, and fuzz tests for parser/runtime robustness.
 
 ## Unit Tests
 
 ### Location
 
 `tests/unit/` — 25 test files (`test_1.cpp` through `test_25.cpp`) plus
 `test_main.cpp`.
 
 ### Framework
 
 Tests use Catch2 v3 when available. If Catch2 is not found, a minimal
 test runner is used instead.
 
 ```cmake
 find_package(Catch2 3 QUIET)
 if(Catch2_FOUND)
   target_link_libraries(ipctk_unit_tests PRIVATE Catch2::Catch2WithMain)
 endif()
 ```
 
 ### Running Tests
 
 ```sh
 cmake -S . -B build -DIPCTK_BUILD_TESTS=ON
 cmake --build build -j
 ctest --test-dir build
 ```
 
 Or run the test binary directly:
 
 ```sh
 ./build/tests/ipctk_unit_tests
 ```
 
 ### Test Categories
 
 | Test File | Coverage Area |
 |---|---|
 | `test_1` – `test_5` | Parser: IPC-L lexing and parsing |
 | `test_6` – `test_10` | Validator: semantic correctness checks |
 | `test_11` – `test_15` | Compiler: lowering and emission |
 | `test_16` – `test_20` | IR: data structure correctness |
 | `test_21` – `test_25` | Integration: end-to-end parse→compile |
 
 ### Writing New Tests
 
 1. Add a new `test_N.cpp` file in `tests/unit/`.
 2. Include `IPCtk.hpp` and the test macros.
 3. Add the file to `tests/CMakeLists.txt`.
 4. Follow the existing pattern:
 
 ```cpp
 #include <ipctk.hpp>
 
 TEST_CASE("parser handles valid pubsub", "[parse]") {
   auto src = R"(socket s = tcp.listen("127.0.0.1:7000");)";
   auto result = ipctk::parse::parse_program(src);
   REQUIRE(result.is_ok());
 }
 ```
 
 ## Fuzz Testing
 
 ### Location
 
 `tests/fuzz/` — 20 fuzz cases (`fuzz_case_1.cpp` through
 `fuzz_case_20.cpp`) plus `fuzz_target.cpp` and seed files
 (`seed_1.txt` through `seed_18.txt`).
 
 ### Purpose
 
 Fuzz tests target the parser and runtime for robustness against:
 
 - Malformed IPC-L source text.
 - Adversarial input patterns (deeply nested expressions, invalid UTF-8).
 - Edge cases in stage chaining and resource references.
 - Memory safety under unexpected IR states.
 
 ### Running Fuzz Tests
 
 ```sh
 cmake --build build --target ipctk_fuzz_target
 ```
 
 The fuzz target is designed to be used with libFuzzer or AFL++:
 
 ```sh
 # With libFuzzer (Clang)
 clang++ -fsanitize=fuzzer,address tests/fuzz/fuzz_target.cpp \
   -I include -o fuzz_ipctk
 ./fuzz_ipctk tests/fuzz/
 ```
 
 ### Seed Files
 
 Seeds provide valid IPC-L examples that the fuzzer mutates. Each seed
 covers a different protocol or edge case:
 
 - `seed_1.txt` — `seed_9.txt`: Valid protocol definitions.
 - `seed_10.txt` — `seed_14.txt`: Edge cases (empty pipes, large resources).
 - `seed_15.txt` — `seed_18.txt`: Malformed but near-valid inputs.
 
 ## Reliability Guarantees
 
 ### Deterministic Output
 
 The parse → validate → compile pipeline is deterministic:
 
 - Same input always produces the same output.
 - No random number generation, no system-clock dependence.
 - Output ordering is stable across runs.
 
 ### Error Handling
 
 - All fallible operations return `Result<T>`.
 - Parse errors include source position information.
 - Validation errors include the name of the offending construct.
 - Compilation errors identify the specific operation without a backend rule.
 
 ### Memory Safety
 
 - No raw `new`/`delete` in the public API.
 - `shared_ptr` for nested steps with automatic cleanup.
 - Stack-allocated IR types with value semantics.
 - No global mutable state.
 
 ### Thread Safety
 
 - The library is reentrant: multiple threads can parse, validate, and
   compile different programs concurrently.
 - The library is not thread-safe for shared mutable state: do not share
   IR objects across threads without external synchronization.
 - Generated code's thread safety depends on the backend and the IPC-L
   graph design (lock/unlock pairing).
 
 ## Failure Interpretation
 
 | Failure | Interpretation | Action |
 |---|---|---|
 | Unit test failure | Semantic regression or contract drift | Check the specific test for expected vs. actual behavior |
 | Fuzz crash | Parser safety or unchecked edge-state transition | Analyze the crashing input; add a regression test |
 | Validation error | Protocol definition violates IPC-L rules | Fix the `.ipcl` file; check the validation message |
 | Compilation error | Missing ITKD rule or capability | Add a rule to the backend or declare the capability |
 | Runtime error | Generated code or runtime environment issue | Check the generated output; verify the runtime setup |
 
 ## Coverage Expectations
 
 - **New public API behavior**: Requires unit test coverage.
 - **Parser/runtime boundary changes**: Require fuzz seed updates.
 - **New IPC-L operations**: Require tests in both unit and fuzz suites.
 - **New ITKD backends**: Require validation tests for the backend spec
   and compatibility tests with known programs.
 - **Protocol additions to stdipc**: Should be accompanied by unit tests
   that parse and validate the `.ipcl` files.
 
 ## Continuous Integration
 
 While IPCtk does not ship with a CI configuration, the recommended CI
 pipeline is:
 
 1. Build with `-DIPCTK_BUILD_TESTS=ON -DIPCTK_BUILD_EXAMPLES=ON`.
 2. Run `ctest` for unit tests.
 3. Run the fuzz target for a fixed duration (e.g., 60 seconds).
 4. Build documentation with `-DIPCTK_BUILD_DOCS=ON`.
 5. Verify install layout with `cmake --install`.
