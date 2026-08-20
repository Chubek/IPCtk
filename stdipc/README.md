 # stdipc: Standard Library of IPC Protocols
 
 `stdipc` is the canonical protocol library for IPCtk. It provides
 ready-to-use IPC-L definitions for every major inter-process communication
 pattern, along with C++ native-DSL counterparts and primitives for
 raw OS-level IPC mechanisms.
 
 ## Design Philosophy
 
 - **Composable**: every protocol is defined as a graph of IPC-L primitives,
   so you can compose them into larger systems.
 - **Deterministic**: validation and lowering rules are applied uniformly;
   every protocol passes through the same IPC-L → ITKD pipeline.
 - **Backend-portable**: all `.ipcl` definitions target the ITKD abstract
   machine — generate C, Python, Ruby, or custom backends without changing
   the protocol source.
 - **Minimal**: each protocol exposes only the stages required for its
   messaging contract; no hidden state or implicit side-effects.
 
 ## Directory Layout
 
 ```
 stdipc/
 ├── README.md                  # This file
 ├── CMakeLists.txt             # Build integration
 ├── protocols/                 # High-level IPC-L protocol definitions
 │   ├── pubsub.ipcl
 │   ├── reqrep.ipcl
 │   ├── survey.ipcl
 │   ├── pushpull.ipcl
 │   ├── messagebus.ipcl
 │   ├── eventbus.ipcl
 │   ├── rpc.ipcl
 │   ├── mailbox.ipcl
 │   └── channel.ipcl
 ├── native/                    # C++ native-DSL headers (one per protocol)
 │   ├── pubsub.hpp
 │   ├── reqrep.hpp
 │   ├── survey.hpp
 │   ├── pushpull.hpp
 │   ├── messagebus.hpp
 │   ├── eventbus.hpp
 │   ├── rpc.hpp
 │   ├── mailbox.hpp
 │   └── channel.hpp
 └── primitives/                # Raw OS IPC primitives expressed in IPC-L
     ├── pipe.ipcl
     ├── fifo.ipcl
     ├── shm.ipcl
     ├── mqueue.ipcl
     ├── tcp_socket.ipcl
     ├── udp_socket.ipcl
     ├── unix_socket.ipcl
     ├── mmap.ipcl
     ├── semaphore.ipcl
     ├── signal.ipcl
     ├── futex.ipcl
     ├── eventfd.ipcl
     └── io_uring.ipcl
 ```
 
 ## High-Level Protocols (`protocols/`)
 
 ### pubsub — Publish/Subscribe
 
 A fan-out messaging pattern. Publishers push messages into topics;
 subscribers receive only the messages for topics they have registered
 interest in. The protocol uses a shared subscription table with a
 semaphore for mutual exclusion, plus a multi-producer/multi-consumer
 ring queue for publication buffering.
 
 - **Resources**: `tcp.listen` sockets, `shm.open` subscription table,
   `semaphore` lock, `mpmc.ring` queue, `eventfd` signal.
 - **Pipes**: `publish_path`, `subscribe_path`, `dispatch_path`.
 - **Capabilities**: `pubsub`.
 
 ### reqrep — Request/Reply
 
 A synchronous RPC-like pattern. A client sends a request and blocks
 until the server replies. The protocol multiplexes requests over a
 single TCP socket pair, using an in-flight request map and a semaphore
 for state synchronization.
 
 - **Resources**: `tcp.listen` and `tcp.connect` sockets, `shm.open`
   request map, `semaphore` lock, `eventfd` arrival signal.
 - **Pipes**: `request_path`, `reply_path`, `dispatch_path`.
 - **Capabilities**: `reqrep`.
 
 ### survey — Surveyor/Respondent
 
 A scatter-gather inquiry pattern. A surveyor broadcasts a question to
 all respondents, collects answers within a deadline, and aggregates
 results. Built on UDP multicast for the survey broadcast and TCP for
 response collection.
 
 - **Resources**: `udp.multicast` survey socket, `tcp.listen` response
   socket, `shm.open` respondent table, `semaphore` lock, `timer` deadline.
 - **Pipes**: `survey_path`, `respond_path`, `collect_path`.
 - **Capabilities**: `survey`.
 
 ### pushpull — Push/Pull
 
 A pipeline/work-distribution pattern. Pushers distribute work items
 across a pool of pullers. The protocol uses a fair-queue dispatcher
 over a shared-memory work queue with round-robin assignment.
 
 - **Resources**: `tcp.listen` push socket, `tcp.listen` pull socket,
   `shm.open` work queue, `semaphore` lock, `eventfd` work-ready signal.
 - **Pipes**: `push_path`, `pull_path`, `distribute_path`.
 - **Capabilities**: `pushpull`.
 
 ### messagebus — Message Bus
 
 A many-to-many routed messaging backbone. Producers post messages to
 named channels; consumers receive from channels they subscribe to.
 Supports content-based routing via a filter stage.
 
 - **Resources**: `tcp.listen` bus socket, `shm.open` channel table,
   `shm.open` subscription index, `semaphore` lock, `mpmc.ring` buffer.
 - **Pipes**: `produce_path`, `consume_path`, `route_path`.
 - **Capabilities**: `pubsub`.
 
 ### eventbus — Event Bus
 
 A fire-and-forget notification backbone. Sources emit typed events;
 sinks register handlers by event type. Events are dispatched
 asynchronously with optional priority ordering.
 
 - **Resources**: `tcp.listen` event socket, `shm.open` handler registry,
   `semaphore` lock, `mpmc.ring` priority queue, `eventfd` event-ready.
 - **Pipes**: `emit_path`, `register_path`, `dispatch_path`.
 - **Capabilities**: `pubsub`.
 
 ### rpc — Remote Procedure Call
 
 A full RPC stack with marshalling, invocation, and return-path
 routing. Calls are serialized with a schema registry, dispatched
 to a worker pool, and results are routed back to the caller.
 
 - **Resources**: `tcp.listen` RPC socket, `shm.open` schema registry,
   `shm.open` call table, `semaphore` lock, `mpmc.ring` worker queue,
   `eventfd` completion signal.
 - **Pipes**: `call_path`, `invoke_path`, `return_path`.
 - **Capabilities**: `reqrep`.
 
 ### mailbox — Mailbox
 
 An asynchronous message-drop pattern. Each participant owns a named
 mailbox; senders deposit messages, and receivers poll or block on
 their mailbox. Supports both blocking and non-blocking receive.
 
 - **Resources**: `tcp.listen` mailbox socket, `shm.open` mailbox
   storage, `semaphore` per-mailbox lock, `eventfd` per-mailbox
   arrival signal.
 - **Pipes**: `send_path`, `receive_path`, `poll_path`.
 - **Capabilities**: `mailbox`.
 
 ### channel — Channel
 
 A bidirectional typed stream between two endpoints. Channels carry
 framed messages with optional back-pressure. Built on Unix domain
 sockets for colocated processes and TCP for remote ones.
 
 - **Resources**: `unix.listen` or `tcp.listen` socket, `eventfd`
   back-pressure signal, `shm.open` buffer window.
 - **Pipes**: `send_path`, `recv_path`, `flow_control_path`.
 - **Capabilities**: `channel`.
 
 ## Primitives (`primitives/`)
 
 Each primitive file models a single OS-level IPC mechanism as a
 minimal IPC-L graph. These are the building blocks that the
 high-level protocols compose.
 
 | Primitive | Description | Platform |
 |---|---|---|
 | `pipe.ipcl` | Unidirectional byte stream between parent and child. | All POSIX |
 | `fifo.ipcl` | Named pipe for unrelated processes. | All POSIX |
 | `shm.ipcl` | Shared memory region with optional locking. | All POSIX |
 | `mqueue.ipcl` | POSIX message queue with priority support. | All POSIX |
 | `tcp_socket.ipcl` | TCP stream socket (listen + connect). | All |
 | `udp_socket.ipcl` | UDP datagram socket (multicast-capable). | All |
 | `unix_socket.ipcl` | Unix domain socket (stream + datagram). | All POSIX |
 | `mmap.ipcl` | Memory-mapped file for shared access. | All POSIX |
 | `semaphore.ipcl` | Named POSIX semaphore for mutual exclusion. | All POSIX |
 | `signal.ipcl` | Asynchronous signal delivery between processes. | All POSIX |
 | `futex.ipcl` | Fast userspace mutex (low-contention locking). | Linux only |
 | `eventfd.ipcl` | File-descriptor based event notification. | Linux only |
 | `io_uring.ipcl` | Async I/O submission/completion ring. | Linux only |
 
 ## Native C++ DSL (`native/`)
 
 Every high-level protocol has a corresponding C++ header that uses
 `ipctk::dsl` to express the same graph in native code. The native
 headers are drop-in: `#include` the header, instantiate the protocol
 object, and call `compile()` with your chosen ITKD backend.
 
 Example usage:
 
 ```cpp
 #include <ipctk.hpp>
 #include "stdipc/native/pubsub.hpp"
 
 auto pubsub = ipctk::stdipc::PubSub("127.0.0.1", 7000, 7001);
 auto result = pubsub.compile<"dest/C.itkd">("pubsub.c");
 ```
 
 ## Capability Matrix
 
 Each protocol declares the ITKD capabilities it requires. When
 selecting a backend, ensure it provides the listed capabilities.
 
 | Protocol | `pubsub` | `reqrep` | `survey` | `pushpull` | `mailbox` | `channel` |
 |---|---|---|---|---|---|---|
 | pubsub | ✓ | | | | | |
 | reqrep | | ✓ | | | | |
 | survey | | | ✓ | | | |
 | pushpull | | | | ✓ | | |
 | messagebus | ✓ | | | | | |
 | eventbus | ✓ | | | | | |
 | rpc | | ✓ | | | | |
 | mailbox | | | | | ✓ | |
 | channel | | | | | | ✓ |
 
 ## Validation
 
 All `.ipcl` files in this library pass IPC-L validation. They satisfy:
 
 - Declaration-before-use for all resources.
 - Chain compatibility (stage types match across pipe transitions).
 - Lock/unlock pairing for shared-state mutation.
 - No unresolved terminal transport stages.
 
 ## Backend Compatibility
 
 The bundled ITKD backends (`dest/C.itkd`, `dest/Python.itkd`,
 `dest/Ruby.itkd`) declare `reqrep` and `pubsub` capabilities by
 default. Protocols requiring `survey`, `pushpull`, `mailbox`, or
 `channel` need a backend that advertises those capabilities.
 
 See the ITKD manual (`docs/ITKD-Manual.md`) for adding capabilities
 to an existing backend or writing a new one.
 
 ## License
 
 Same as IPCtk. See the root `LICENSE` file.
