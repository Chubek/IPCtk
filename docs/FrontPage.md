 # IPCtk Front Page
 
 IPCtk provides a header-only C++ IPC toolkit with IPC-L semantic flow
 specification, ITKD target mapping/generation, and a standard library
 of composable IPC protocols.
 
 ## Quick Links
 
 | Category | Link |
 |---|---|
 | IPC-L standalone manual | [docs/IPC-L-Manual.md](IPC-L-Manual.md) |
 | ITKD standalone manual | [docs/ITKD-Manual.md](ITKD-Manual.md) |
 | Standard Library (stdipc) | [stdipc/README.md](../stdipc/README.md) |
 
 ## Manual (16 Chapters)
 
 | Chapter | Title | Description |
 |---|---|---|
 | 1 | [Project Overview](manual/Chapter-1.md) | Architecture, components, design principles |
 | 2 | [Build and Installation](manual/Chapter-2.md) | CMake workflow, options, platform support |
 | 3 | [Core C++ API](manual/Chapter-3.md) | IR types, parser, validator, compiler APIs |
 | 4 | [IPC-L Language Reference](manual/Chapter-4.md) | Full syntax, resource types, stage reference |
 | 5 | [ITKD Language Reference](manual/Chapter-5.md) | Backend format, rules, type mappings |
 | 6 | [Bindings and Integration](manual/Chapter-6.md) | SWIG, XFeats, integration patterns |
 | 7 | [Testing, Fuzzing, Reliability](manual/Chapter-7.md) | Unit tests, fuzz harness, CI pipeline |
 | 8 | [Developer Guide](manual/Chapter-8.md) | Standards, change process, release checklist |
 | 9 | [Standard Library (stdipc)](manual/Chapter-9.md) | Protocol catalog, usage, composition |
 | 10 | [IPC Primitives Reference](manual/Chapter-10.md) | All 13 primitives, platform availability |
 | 11 | [Native C++ DSL Guide](manual/Chapter-11.md) | Embedded DSL, builders, type safety |
 | 12 | [Protocol Design Patterns](manual/Chapter-12.md) | 8 patterns, selection guide, anti-patterns |
 | 13 | [Backend Authoring](manual/Chapter-13.md) | Custom ITKD backends, testing, distribution |
 | 14 | [Performance and Optimization](manual/Chapter-14.md) | Primitive selection, sync, serialization |
 | 15 | [Security Considerations](manual/Chapter-15.md) | Threat model, mitigations, secure config |
 | 16 | [Porting and Platform Notes](manual/Chapter-16.md) | Linux, macOS, FreeBSD, Windows, porting guide |
 
 ## API and Sources
 
 | Resource | Path | Description |
 |---|---|---|
 | C++ API | `include/IPCtk.hpp` | Header-only core API |
 | DSL Utilities | `DSLUtils.hpp` | Parser combinators and Result type |
 | Standard Library | `stdipc/` | Protocols, primitives, native headers |
 | Unit Tests | `tests/unit/` | Deterministic regression tests |
 | Fuzz Tests | `tests/fuzz/` | Parser/runtime robustness |
 | Backend Templates | `dest/` | C, Python, Ruby ITKD backends |
 | Bindings | `bindings/` | SWIG interface and XFeats |
