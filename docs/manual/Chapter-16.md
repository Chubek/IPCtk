 # Chapter 16: Porting and Platform Notes
 
 IPCtk is designed for portability, but some primitives are
 platform-specific. This chapter covers platform differences and porting
 guidance.
 
 ## Platform Support Summary
 
 | Platform | POSIX Primitives | Linux-Specific | Notes |
 |---|---|---|---|
 | Linux (x86_64) | All | All | Primary development platform |
 | Linux (ARM64) | All | All | Kernel ≥ 5.1 for io_uring |
 | macOS | All | None | No futex, eventfd, io_uring |
 | FreeBSD | All | None | POSIX primitives only |
 | Windows (WSL) | All | All | Linux kernel features via WSL2 |
 | Windows (Cygwin) | Most | None | POSIX emulation layer |
 | Windows (native) | None | None | Requires porting layer |
 
 ## Linux
 
 Linux is the primary development platform and has the fullest feature
 support.
 
 ### Kernel Requirements
 
 | Feature | Minimum Kernel |
 |---|---|
 | POSIX IPC | 2.6+ |
 | eventfd | 2.6.22+ |
 | futex | 2.6+ |
 | io_uring | 5.1+ |
 | signalfd | 2.6.22+ |
 | timerfd | 2.6.25+ |
 | memfd_create | 3.17+ |
 
 ### Compiler Requirements
 
 - GCC ≥ 11 (C++20 support).
 - Clang ≥ 14 (C++20 support).
 - `libstdc++` or `libc++` with C++20 support.
 
 ### Build Flags
 
 ```sh
 cmake -S . -B build \
   -DCMAKE_CXX_COMPILER=g++-13 \
   -DCMAKE_CXX_STANDARD=20 \
   -DCMAKE_BUILD_TYPE=Release
 ```
 
 ## macOS
 
 macOS supports all POSIX primitives but lacks Linux-specific features.
 
 ### Unavailable Features
 
 - `futex` — use `semaphore` or `dispatch_semaphore` instead.
 - `eventfd` — use `pipe` or `semaphore` instead.
 - `io_uring` — use `kqueue` or `dispatch_io` instead.
 - `signalfd` — use `kqueue` with `EVFILT_SIGNAL` instead.
 - `timerfd` — use `dispatch_source_timer` instead.
 
 ### Compiler
 
 - Apple Clang (Xcode ≥ 14) or Homebrew GCC.
 - C++20 support is available in recent Xcode versions.
 
 ### Build Flags
 
 ```sh
 cmake -S . -B build \
   -DCMAKE_CXX_COMPILER=clang++ \
   -DCMAKE_CXX_STANDARD=20 \
   -DCMAKE_BUILD_TYPE=Release
 ```
 
 ## FreeBSD
 
 FreeBSD supports POSIX primitives. Some Linux-specific features have
 FreeBSD equivalents.
 
 ### Alternatives
 
 | Linux Feature | FreeBSD Alternative |
 |---|---|
 | `eventfd` | `pipe` or `kqueue` EVFILT_USER |
 | `futex` | `_umtx_op` |
 | `io_uring` | `aio_*` or custom `kqueue` |
 | `signalfd` | `kqueue` EVFILT_SIGNAL |
 | `timerfd` | `kqueue` EVFILT_TIMER |
 
 ## Windows
 
 Windows has no native POSIX IPC support. Three approaches are available:
 
 ### WSL2 (Recommended)
 
 Full Linux kernel, all features available. Use the Linux build
 instructions.
 
 ### Cygwin
 
 POSIX emulation layer. Most POSIX primitives work, but Linux-specific
 features (`futex`, `eventfd`, `io_uring`) are unavailable.
 
 ### Native Windows
 
 Requires a porting layer that maps IPC-L primitives to Windows APIs:
 
 | IPC-L Primitive | Windows API |
 |---|---|
 | `pipe` | `CreatePipe` |
 | `fifo` | `CreateNamedPipe` |
 | `shm` | `CreateFileMapping` + `MapViewOfFile` |
 | `mmap` | `CreateFileMapping` + `MapViewOfFile` |
 | `semaphore` | `CreateSemaphore` |
 | `signal` | Not directly available; use `Event` objects |
 | `tcp_socket` | Winsock2 |
 | `udp_socket` | Winsock2 |
 | `unix_socket` | Supported in Windows 10 build 17063+ |
 | `mqueue` | Not directly available; use `MSMQ` or custom |
 
 ## Porting Guide
 
 ### Adding a New Platform
 
 1. Identify which IPC-L primitives are available on the platform.
 2. Create a new ITKD backend (`dest/<platform>.itkd`) that maps
    operations to platform-specific APIs.
 3. For unavailable primitives, either:
    - Provide an emulation layer using available primitives.
    - Mark the primitive as unsupported in the backend's capabilities.
 4. Test the backend with the standard library protocols.
 5. Document platform-specific build instructions.
 
 ### Conditional Compilation
 
 When writing platform-specific code in the C++ API or native DSL headers,
 use preprocessor guards:
 
 ```cpp
 #ifdef __linux__
   // Linux-specific code
 #elif defined(__APPLE__)
   // macOS-specific code
 #elif defined(__FreeBSD__)
   // FreeBSD-specific code
 #elif defined(_WIN32)
   // Windows-specific code
 #endif
 ```
 
 ### Feature Detection
 
 At build time, detect available features:
 
 ```cmake
 include(CheckIncludeFile)
 include(CheckFunctionExists)
 
 check_include_file(sys/eventfd.h HAVE_EVENTFD)
 check_include_file(linux/futex.h HAVE_FUTEX)
 check_include_file(liburing.h HAVE_IO_URING)
 
 if(HAVE_EVENTFD)
   target_compile_definitions(ipctk INTERFACE IPCTK_HAS_EVENTFD=1)
 endif()
 ```
 
 At runtime, IPCtk uses `#if __has_include(...)` for feature detection:
 
 ```cpp
 #if __has_include(<sys/eventfd.h>)
   // eventfd available
 #endif
 ```
 
 ## Cross-Platform Protocol Design
 
 When designing protocols that must work across platforms:
 
 1. **Use only POSIX primitives** for maximum portability:
    `pipe`, `fifo`, `shm`, `semaphore`, `tcp_socket`, `unix_socket`.
 
 2. **Avoid Linux-specific primitives** unless you can provide fallbacks:
    `futex` → `semaphore`, `eventfd` → `pipe`, `io_uring` → `aio`.
 
 3. **Declare appropriate capabilities** in your backend:
    Only declare `pubsub` and `reqrep` if you support all operations
    those protocols use.
 
 4. **Test on all target platforms** before releasing.
 
 5. **Document platform requirements** in your protocol's comments.
 
 ## Platform-Specific Backend Examples
 
 ### Linux-Optimized Backend
 
 ```
 target linux-optimized
 capability pubsub
 capability reqrep
 capability pushpull
 include <sys/eventfd.h>
 include <linux/futex.h>
 include <liburing.h>
 map string -> const char*
 map bytes -> uint8_t*
 rule lock_optimized = template("futex_wait(&futex_word, 1);")
 rule unlock_optimized = template("futex_wake(&futex_word, 1);")
 ```
 
 ### Portable POSIX Backend
 
 ```
 target posix
 capability pubsub
 capability reqrep
 include <semaphore.h>
 include <sys/mman.h>
 map string -> const char*
 map bytes -> uint8_t*
 rule lock_portable = template("sem_wait(&sem);")
 rule unlock_portable = template("sem_post(&sem);")
 ```
 
 ## Troubleshooting Platform Issues
 
 | Symptom | Likely Cause | Solution |
 |---|---|---|
 | `eventfd` not found | macOS or FreeBSD | Use `pipe` or `semaphore` |
 | `futex` not found | Non-Linux platform | Use `semaphore` |
 | `io_uring` not found | Kernel < 5.1 or non-Linux | Use `aio` or synchronous I/O |
 | `sem_open` fails | Permission denied | Check `/dev/shm` permissions |
 | `shm_open` fails | `tmpfs` not mounted | Mount `tmpfs` on `/dev/shm` |
 | `unix.listen` fails | Path too long | Use shorter socket path |
 | `tcp.listen` fails | Port in use | Choose a different port |
 | SWIG not found | Not installed | Install SWIG ≥ 4.0 |
 | C++20 not supported | Old compiler | Upgrade GCC/Clang |
