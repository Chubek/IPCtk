 # Chapter 8: Developer Guide and Contribution
 
 This chapter covers the development workflow, coding standards, and
 release process for IPCtk contributors.
 
 ## Development Environment
 
 ### Recommended Setup
 
 ```sh
 git clone https://github.com/Chubek/IPCtk.git
 cd IPCtk
 cmake -S . -B build -DIPCTK_BUILD_TESTS=ON -DIPCTK_BUILD_EXAMPLES=ON
 cmake --build build -j$(nproc)
 ```
 
 ### Editor Integration
 
 - Use `compile_commands.json` for LSP support:
   ```sh
   cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   ln -s build/compile_commands.json .
   ```
 - The project uses `.clang-format` for C++ formatting.
 
 ## Coding Standards
 
 ### API Stability
 
 - Preserve header-only API stability unless version-gated.
 - Breaking changes to the IR, parser, or compiler API require a major
   version bump.
 - Additions to the standard library (`stdipc/`) are backward-compatible
   and considered minor changes.
 
 ### DSL Separation
 
 - Keep IPC-L semantics separate from target-specific lowering.
 - Do not add target-language assumptions to the IR or validator.
 - All target-specific behavior belongs in ITKD backends.
 
 ### Code Style
 
 - Follow the existing style in `IPCtk.hpp`:
   - `snake_case` for functions and variables.
   - `PascalCase` for types and structs.
   - `inline` for all functions in the header.
   - `auto` for return type deduction where appropriate.
   - Namespaces: `ipctk`, `ipctk::ir`, `ipctk::parse`, `ipctk::validate`.
 
 ### Header Organization
 
 - Public API goes in `IPCtk.hpp`.
 - DSL utilities go in `DSLUtils.hpp`.
 - Standard library headers go in `stdipc/native/`.
 - Do not add new public headers without updating the CMake install rules.
 
 ### Error Handling
 
 - Use `ipctk::Result<T>` for all fallible operations.
 - Include context in error messages (source position, construct name).
 - Never throw exceptions from the public API.
 - Never assert on user input; return an error instead.
 
 ## Change Process
 
 ### Adding a Feature
 
 1. **Design**: Document the feature in the relevant manual chapter.
 2. **Implement**: Add the code to `IPCtk.hpp` or `stdipc/`.
 3. **Test**: Add unit tests in `tests/unit/`.
 4. **Fuzz**: Add fuzz seeds if the feature touches the parser.
 5. **Document**: Update the manual and any affected `.md` files.
 6. **Backend**: Update `dest/*.itkd` if new operations require rules.
 
 ### Modifying the IR
 
 1. Update the struct definitions in `ipctk::ir`.
 2. Update the parser (`ipctk::parse`) to produce the new IR shape.
 3. Update the validator (`ipctk::validate`) to check the new fields.
 4. Update the compiler (`ipctk::compile`) to handle the new fields.
 5. Update all ITKD backends if the change affects emission.
 6. Update all `.ipcl` files in `stdipc/` if the syntax changes.
 7. Update unit and fuzz tests.
 
 ### Modifying an ITKD Backend
 
 1. Edit the `.itkd` file in `dest/`.
 2. Validate the backend spec:
   ```cpp
   auto spec = ipctk::parse::parse_backend_file("dest/C.itkd");
   auto valid = ipctk::validate::validate(spec.unwrap());
   ```
 3. Test with known programs from `stdipc/protocols/`.
 4. Update the ITKD manual if the feature is generally applicable.
 
 ### Adding a Standard Protocol
 
 1. Create the `.ipcl` file in `stdipc/protocols/`.
 2. Create the corresponding `.hpp` file in `stdipc/native/`.
 3. Update `stdipc/README.md` with the protocol documentation.
 4. Add a unit test that parses the `.ipcl` file.
 5. If the protocol requires a new capability, update the capability
    matrix and add the capability to bundled backends.
 
 ## Directory Structure Rules
 
 | Path | What Belongs | What Does Not |
 |---|---|---|
 | `include/` | Public headers (`IPCtk.hpp`, `DSLUtils.hpp`) | Implementation details |
 | `stdipc/` | Standard library protocols and primitives | Application-specific protocols |
 | `dest/` | ITKD backend templates | Generated output |
 | `bindings/` | SWIG interface and XFeats | Language-specific runtime code |
 | `tests/unit/` | Deterministic regression tests | Integration or performance tests |
 | `tests/fuzz/` | Fuzz harnesses and seeds | Unit tests |
 | `docs/` | Manual, Doxygen config, front page | Generated documentation |
 | `cmake/` | Build system config | Build artifacts |
 | `third_party/` | Vendored dependencies (Polyflow) | Modified dependency code |
 
 ## Release Checklist
 
 Before tagging a release:
 
 - [ ] Clean configure and build: `cmake -S . -B build && cmake --build build -j`
 - [ ] All tests pass: `ctest --test-dir build`
 - [ ] Documentation generates: `cmake --build build --target docs`
 - [ ] Install layout verified: `cmake --install build --prefix /tmp/ipctk-install`
 - [ ] pkg-config file validated: `pkg-config --cflags --libs ipctk`
 - [ ] Bindings generate (if enabled): `cmake --build build --target bindings-python`
 - [ ] All `.ipcl` files in `stdipc/` parse and validate successfully.
 - [ ] `README.md` and `stdipc/README.md` are up to date.
 - [ ] `FrontPage.md` chapter table is current.
 - [ ] Version string in `CMakeLists.txt` matches the tag.
 
 ## Commit Conventions
 
 - Use imperative mood: "Add X", "Fix Y", "Update Z".
 - Prefix with the affected area: `ir:`, `parse:`, `validate:`, `compile:`,
   `stdipc:`, `docs:`, `bindings:`, `tests:`, `build:`.
 
 Examples:
 
 ```
 stdipc: add mailbox protocol definition
 parse: fix comment stripping in multi-line strings
 docs: expand Chapter 4 with full stage reference
 tests: add fuzz seed for nested pipe expressions
 ```
 
 ## Getting Help
 
 - **Issues**: File bugs and feature requests on the GitHub issue tracker.
 - **Discussions**: Use GitHub Discussions for questions and design proposals.
 - **Documentation**: Start with this manual and the standalone
   `IPC-L-Manual.md` and `ITKD-Manual.md`.
 - **Examples**: The `stdipc/` directory contains complete, validated
   protocol definitions that serve as reference implementations.
 
 ## Code of Conduct
 
 - Be respectful and constructive in all interactions.
 - Focus on the technical merits of proposals.
 - Assume good intent from other contributors.
 - Follow the existing code style and conventions.
 - Document your changes thoroughly.
