 # Chapter 2: Build and Installation
 
 ## Prerequisites
 
 - **CMake** ≥ 3.16
 - **C++ compiler** with C++20 support (GCC ≥ 11, Clang ≥ 14, MSVC ≥ 2022)
 - **Doxygen** (optional, for documentation generation)
 - **Python 3** (optional, for SWIG bindings)
 - **Catch2 v3** (optional, for unit test execution)
 
 ## Quick Start
 
 ```sh
 git clone https://github.com/Chubek/IPCtk.git
 cd IPCtk
 cmake -S . -B build
 cmake --build build -j
 cmake --install build --prefix /usr/local
 ```
 
 ## CMake Workflow
 
 The project follows standard CMake conventions:
 
 - **Out-of-source builds**: `cmake -S . -B build`
 - **Build**: `cmake --build build -j$(nproc)`
 - **Install**: `cmake --install build --prefix <prefix>`
 
 ### Build Options
 
 | Option | Default | Description |
 |---|---|---|
 | `IPCTK_BUILD_EXAMPLES` | `ON` | Build C++ example programs |
 | `IPCTK_BUILD_TESTS` | `ON` | Build unit and fuzz test targets |
 | `IPCTK_BUILD_DOCS` | `ON` | Enable Doxygen documentation target |
 | `IPCTK_ENABLE_BINDINGS` | `ON` | Enable SWIG bindings generation targets |
 
 Set options on the command line:
 
 ```sh
 cmake -S . -B build -DIPCTK_BUILD_TESTS=OFF
 ```
 
 ## Install Outputs
 
 After installation, the following files are placed under the prefix:
 
 | Output | Destination |
 |---|---|
 | `IPCtk.hpp`, `DSLUtils.hpp` | `${CMAKE_INSTALL_INCLUDEDIR}` |
 | `stdipc/native/*.hpp` | `${CMAKE_INSTALL_INCLUDEDIR}/stdipc/native` |
 | `dest/*.itkd` | `${CMAKE_INSTALL_DATADIR}/stdipc/dest` |
 | `stdipc/protocols/*.ipcl` | `${CMAKE_INSTALL_DATADIR}/stdipc/protocols` |
 | `stdipc/primitives/*.ipcl` | `${CMAKE_INSTALL_DATADIR}/stdipc/primitives` |
 | `stdipc/README.md` | `${CMAKE_INSTALL_DATADIR}/stdipc` |
 | `ipctk.pc` | `${CMAKE_INSTALL_LIBDIR}/pkgconfig` |
 | CMake package config | `${CMAKE_INSTALL_LIBDIR}/cmake/IPCtk` |
 
 ## Using IPCtk in Your Project
 
 ### Via CMake `find_package`
 
 ```cmake
 find_package(IPCtk REQUIRED)
 target_link_libraries(my_app PRIVATE IPCtk::ipctk)
 target_link_libraries(my_app PRIVATE IPCtk::stdipc)  # optional
 ```
 
 ### Via pkg-config
 
 ```sh
 pkg-config --cflags ipctk
 pkg-config --libs ipctk
 ```
 
 ### Via Direct Include
 
 Since IPCtk is header-only, you can also copy `IPCtk.hpp` and
 `DSLUtils.hpp` directly into your project and include them.
 
 ```cpp
 #include "IPCtk.hpp"
 ```
 
 ## Documentation
 
 - Doxygen config source: `docs/Doxyfile.in`
 - Build docs: `cmake --build build --target docs`
 - Output: `build/docs/html/index.html`
 - The main page is generated from `docs/FrontPage.md`.
 
 ## Platform Support
 
 | Platform | Status | Notes |
 |---|---|---|
 | Linux (x86_64) | Full | All primitives including futex, eventfd, io_uring |
 | Linux (ARM64) | Full | Kernel ≥ 5.1 for io_uring |
 | macOS | Partial | No futex, eventfd, io_uring; POSIX primitives only |
 | FreeBSD | Partial | POSIX primitives; no Linux-specific features |
 | Windows | Experimental | POSIX primitives via WSL or Cygwin |
 
 ## Troubleshooting
 
 ### Link errors in examples
 
 Usually indicate a missing `main` function or stale build artifacts.
 Run `cmake --build build --clean-first` and rebuild.
 
 ### Include resolution issues
 
 If the compiler cannot find `IPCtk.hpp`, ensure:
 
 - The include directory is on the search path (`-I` flag).
 - The header is installed (`cmake --install`).
 - For out-of-tree consumption, `find_package(IPCtk)` is configured.
 
 ### C++20 compatibility
 
 IPCtk requires C++20 for concepts, spans, and `std::string_view`. Ensure
 your compiler is set to C++20 mode:
 
 ```cmake
 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)
 ```
 
 ### Polyflow not found
 
 If `IPCTK_HAS_POLYFLOW` is 0, the library still functions but DSL
 features that depend on Polyflow's graph processing will be disabled.
 Ensure `third_party/Polyflow` is present or install Polyflow separately.
