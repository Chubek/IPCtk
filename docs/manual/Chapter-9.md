 # Chapter 9: Standard Library of Protocols (stdipc)
 
 The `stdipc` directory contains the canonical protocol library for IPCtk.
 It provides ready-to-use IPC-L definitions for every major IPC pattern,
 along with C++ native-DSL counterparts.
 
 ## Overview
 
 The standard library is organized into three subdirectories:
 
 | Directory | Content | Format |
 |---|---|---|
 | `protocols/` | High-level protocol definitions | `.ipcl` (IPC-L syntax) |
 | `primitives/` | Raw OS IPC primitives | `.ipcl` (IPC-L syntax) |
 | `native/` | C++ native-DSL headers | `.hpp` (C++20) |
 
 ## High-Level Protocols
 
 ### pubsub — Publish/Subscribe
 
 A fan-out messaging pattern with topic-based routing. Publishers push
 messages into topics; subscribers receive only messages for topics they
 have registered interest in.
 
 **Resources**: TCP listen sockets, shared subscription table, semaphore
 lock, MPMC ring queue, eventfd signal.
 
 **Pipes**: `publish_path`, `subscribe_path`, `dispatch_path`.
 
 **Usage**:
 ```cpp
 #include "stdipc/native/pubsub.hpp"
 auto [pub_in, sub_in, table, lock, queue, signal, pub, sub, dispatch] =
   ipctk::stdipc::build_pubsub("127.0.0.1", 7000, 7001);
 ```
 
 ### reqrep — Request/Reply
 
 A synchronous RPC-like pattern. A client sends a request and blocks
 until the server replies. Uses an in-flight request map for multiplexing.
 
 **Resources**: TCP listen/connect sockets, shared request map, semaphore
 lock, eventfd signal.
 
 **Pipes**: `request_path`, `reply_path`, `dispatch_path`.
 
 ### survey — Surveyor/Respondent
 
 A scatter-gather inquiry pattern. A surveyor broadcasts a question,
 collects answers within a deadline, and aggregates results.
 
 **Resources**: UDP multicast socket, TCP response socket, shared
 respondent table, semaphore lock, timer.
 
 **Pipes**: `survey_path`, `respond_path`, `collect_path`.
 
 ### pushpull — Push/Pull
 
 A pipeline/work-distribution pattern. Pushers distribute work items
 across a pool of pullers using round-robin assignment.
 
 **Resources**: TCP push/pull sockets, shared work queue, semaphore lock,
 eventfd signal, round-robin counter.
 
 **Pipes**: `push_path`, `pull_path`, `distribute_path`.
 
 ### messagebus — Message Bus
 
 A many-to-many routed messaging backbone. Producers post to named
 channels; consumers receive from subscribed channels with content-based
 routing.
 
 **Resources**: TCP socket, shared channel table, shared subscription
 index, semaphore lock, MPMC ring buffer, eventfd signal.
 
 **Pipes**: `produce_path`, `consume_path`, `route_path`.
 
 ### eventbus — Event Bus
 
 A fire-and-forget notification backbone. Sources emit typed events;
 sinks register handlers by event type. Events are dispatched
 asynchronously with priority ordering.
 
 **Resources**: TCP socket, shared handler registry, semaphore lock,
 MPMC priority queue, eventfd signal.
 
 **Pipes**: `emit_path`, `register_path`, `dispatch_path`.
 
 ### rpc — Remote Procedure Call
 
 A full RPC stack with marshalling, invocation, and return-path routing.
 Calls are serialized with a schema registry and dispatched to a worker
 pool.
 
 **Resources**: TCP socket, shared schema registry, shared call table,
 semaphore lock, MPMC worker queue, eventfd signal.
 
 **Pipes**: `call_path`, `invoke_path`, `return_path`.
 
 ### mailbox — Mailbox
 
 An asynchronous message-drop pattern. Each participant owns a named
 mailbox; senders deposit and receivers poll or block.
 
 **Resources**: TCP socket, shared mailbox storage, semaphore lock,
 eventfd signal.
 
 **Pipes**: `send_path`, `receive_path`, `poll_path`.
 
 ### channel — Channel
 
 A bidirectional typed stream with framed messages and back-pressure.
 Built on Unix domain sockets for colocated processes.
 
 **Resources**: Unix listen socket, eventfd signal, shared buffer window,
 semaphore lock.
 
 **Pipes**: `send_path`, `recv_path`, `flow_control_path`.
 
 ## Capability Matrix
 
 Each protocol requires specific ITKD capabilities. When selecting a
 backend, ensure it declares the required capabilities.
 
 | Protocol | Required Capability |
 |---|---|
 | pubsub | `pubsub` |
 | messagebus | `pubsub` |
 | eventbus | `pubsub` |
 | reqrep | `reqrep` |
 | rpc | `reqrep` |
 | survey | `survey` |
 | pushpull | `pushpull` |
 | mailbox | `mailbox` |
 | channel | `channel` |
 
 ## Using Protocols
 
 ### From IPC-L Syntax
 
 ```cpp
 #include <ipctk.hpp>
 
 auto program = ipctk::parse::parse_program_file(
   "stdipc/protocols/pubsub.ipcl");
 auto backend = ipctk::parse::parse_backend_file("dest/C.itkd");
 auto result  = ipctk::compile(program.unwrap(), backend.unwrap());
 std::cout << result.unwrap() << "\n";
 ```
 
 ### From Native DSL
 
 ```cpp
 #include <ipctk.hpp>
 #include "stdipc/native/pubsub.hpp"
 
 auto [pub_in, sub_in, table, lock, queue, signal, pub, sub, dispatch] =
   ipctk::stdipc::build_pubsub("127.0.0.1", 7000, 7001);
 ```
 
 ## Composing Protocols
 
 Protocols can be composed by combining their resources and pipes into a
 single `ir::Program`. For example, combining pubsub and reqrep to build
 a service that both publishes events and handles requests:
 
 ```cpp
 auto [pub_res..., pub_pipes...] = ipctk::stdipc::build_pubsub("127.0.0.1", 7000, 7001);
 auto [req_res..., req_pipes...] = ipctk::stdipc::build_reqrep("127.0.0.1", 8000);
 
 // Combine into a single program
 // (resources and pipes from both protocols)
 ```
 
 ## Extending the Library
 
 To add a new protocol to the standard library:
 
 1. Create `stdipc/protocols/<name>.ipcl` following the IPC-L syntax.
 2. Create `stdipc/native/<name>.hpp` with a `build_<name>()` function.
 3. Update `stdipc/README.md` with the protocol's documentation.
 4. Add the protocol to the capability matrix.
 5. If the protocol requires a new capability, add it to the bundled
    ITKD backends in `dest/`.
