 # Chapter 1: Project Overview
 
 IPCtk is a header-only, garbage-free C++ library providing a toolkit for
 inter-process communication. It offers two complementary DSLs: IPC-L for
 describing protocol graphs, and ITKD for mapping those graphs to target
 languages. The project is designed to be minimal, deterministic, and
 backend-portable.
 
 ## Architecture
 
 IPCtk is organized into three conceptual layers:
 
 1. **Core API** (`IPCtk.hpp`): The header-only C++ surface that provides
    IR data structures, parsers, validators, and the compile pipeline. It
    also includes the native C++ DSL for constructing protocol graphs
    directly in code.
 
 2. **IPC-L**: The protocol/flow definition language. IPC-L exists in two
    forms — a syntax-based DSL parsed from `.ipcl` files, and a C++ native
    DSL using the `ipctk::dsl` namespace. Both forms produce the same
    intermediate representation.
 
 3. **ITKD**: The backend mapping and generation language. ITKD files
    (`.itkd`) define how IPC-L semantic constructs map to target-language
    code — C, Python, Ruby, or custom backends.
 
 ## Components
 
 | Component | Path | Description |
 |---|---|---|
 | Core API | `include/IPCtk.hpp` | Public C++ API surface, IR types, parser, compiler |
 | DSL Utilities | `DSLUtils.hpp` | Parser-combinator and result-type utilities |
 | IPC-L Manual | `docs/IPC-L-Manual.md` | Standalone IPC-L language reference |
 | ITKD Manual | `docs/ITKD-Manual.md` | Standalone ITKD language reference |
 | Manual | `docs/manual/` | This multi-chapter guide |
 | Standard Library | `stdipc/` | Canonical protocol definitions (`.ipcl` and `.hpp`) |
 | Backend Templates | `dest/` | `C.itkd`, `Python.itkd`, `Ruby.itkd` |
 | Bindings | `bindings/` | SWIG interface and `XFeats` generation |
 | Unit Tests | `tests/unit/` | Deterministic regression tests |
 | Fuzz Tests | `tests/fuzz/` | Parser/runtime robustness harnesses |
 | Build Config | `cmake/` | CMake package config and `ipctk.pc.in` |
 
 ## Relationship Model
 
 ```
 IPC-L (.ipcl or C++ DSL)
        │
        ▼
   IR (Program)
        │
        ▼
   Validation ───► ITKD (.itkd backend spec)
        │                    │
        ▼                    ▼
   Lowering ──────────► Compilation
                            │
                            ▼
                     Target Output
                   (C / Python / Ruby)
 ```
 
 - IPC-L describes semantic transport and process graphs.
 - The IR is the canonical intermediate representation shared by both DSLs.
 - ITKD maps semantic graphs to target-specific artifacts.
 - `dest/*.itkd` defines per-language rendering contracts.
 - The C++ API and DSL flows must remain behaviorally aligned.
 
 ## Design Principles
 
 - **Header-only**: No compilation step for the library itself; consumers
   include `IPCtk.hpp` and go.
 - **Garbage-free**: No dynamic memory allocation in the hot path; all
   resources are stack-allocated or use fixed-size buffers.
 - **Deterministic**: The parse → validate → lower → compile pipeline
   produces the same output for the same inputs, every time.
 - **Backend-portable**: Protocols written in IPC-L are not tied to any
   target language; switch backends without changing the protocol source.
 - **Composable**: Protocols are graphs of primitive IPC operations;
   compose them to build larger systems.
 
 ## Circular Dependency with Polyflow
 
 IPCtk depends on Polyflow (for DSL utilities and graph processing), and
 Polyflow depends on IPCtk (for its own IPC needs). The root directory is
 mirrored in `third_party/Polyflow/IPCtk`. Since the APIs are decoupled,
 this creates a co-independent state: each library is exposed to the other
 through its API, but their APIs are entirely separate.
 
 ## Audience
 
 - **Integrators** building IPC services with pre-built protocol patterns.
 - **Language/binding maintainers** adding new ITKD backends.
 - **Backend template authors** writing `.itkd` mapping rules.
 - **Protocol designers** composing IPC-L graphs for custom workflows.
 - **Verification and fuzzing engineers** testing parser/runtime robustness.
 
 ## Versioning
 
 IPCtk follows semantic versioning. The current version is 0.1.0. Breaking
 changes to the IR, parser API, or ITKD format will increment the major
 version. Backend template additions and new protocol definitions in
 `stdipc` are considered minor or patch changes.
