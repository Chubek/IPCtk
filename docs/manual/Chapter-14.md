 # Chapter 14: Performance and Optimization
 
 IPCtk is designed to be minimal and garbage-free, but protocol design
 choices significantly impact runtime performance. This chapter covers
 optimization strategies for IPC-L graphs.
 
 ## Performance Model
 
 IPC-L graphs are compiled to target code. The performance of the
 generated code depends on:
 
 1. **Primitive choice**: Different IPC primitives have different
    throughput and latency characteristics.
 2. **Synchronization cost**: Lock contention and signal wakeups add
    overhead.
 3. **Serialization cost**: Encode/decode stages consume CPU for
    marshalling.
 4. **Memory access**: Shared memory access patterns affect cache
    performance.
 5. **Backend quality**: The ITKD backend's emission quality affects
    generated code performance.
 
 ## Primitive Selection Guide
 
 ### Throughput (large messages)
 
 | Primitive | Relative Throughput | Notes |
 |---|---|---|
 | Shared memory (`shm`) | Highest | Zero-copy, but requires synchronization |
 | Memory-mapped file (`mmap`) | High | OS-managed paging, good for large datasets |
 | Unix domain socket | High | Kernel bypass on some platforms |
 | Pipe | Medium | Kernel buffer limits apply |
 | TCP socket | Medium | Kernel networking stack overhead |
 | Message queue (`mqueue`) | Low | Per-message kernel transitions |
 | UDP socket | Variable | Depends on datagram size and network |
 
 ### Latency (small messages)
 
 | Primitive | Relative Latency | Notes |
 |---|---|---|
 | Futex | Lowest | Userspace when uncontended |
 | Eventfd | Very low | Single `read`/`write` syscall |
 | Unix domain socket | Low | No network stack overhead |
 | Pipe | Low | Simple kernel buffer |
 | TCP socket (localhost) | Medium | Loopback optimization helps |
 | Semaphore | Medium | Kernel transition per operation |
 | Signal | High | Context switch to handler |
 
 ## Synchronization Optimization
 
 ### Minimize Lock Contention
 
 - Use fine-grained locks: lock only the specific data being accessed,
   not a global lock.
 - Keep critical sections short: do only the minimum work inside
   `lock`/`unlock` pairs.
 - Use read-write patterns: `peek` instead of `retrieve` + `insert` when
   only reading.
 
 **Bad**:
 ```
 pipe slow_path =
     lock(global_lock)
     -> decode(message)       // slow: decoding inside lock
     -> update(table)         // slow: large table update inside lock
     -> encode(response)      // slow: encoding inside lock
     -> unlock(global_lock);
 ```
 
 **Good**:
 ```
 pipe fast_path =
     recv(socket)
     -> decode(message)       // decode outside lock
     -> lock(table_lock)
     -> update(table)         // only the mutation inside lock
     -> unlock(table_lock)
     -> encode(response)      // encode outside lock
     -> send(socket);
 ```
 
 ### Use Signals Instead of Polling
 
 - Use `wait(signal)` / `notify(signal)` for producer-consumer
   synchronization.
 - Avoid busy-waiting or polling loops.
 - `eventfd` is faster than `semaphore` for simple notification.
 
 ### Batch Operations
 
 - Batch multiple messages into a single lock acquisition.
 - Use `io_uring` for batched async I/O on Linux.
 - Accumulate messages in a queue and process them in batches.
 
 ## Serialization Optimization
 
 ### Choose Efficient Formats
 
 | Format | Size | Speed | Use Case |
 |---|---|---|---|
 | Raw bytes | Minimal | Fastest | Homogeneous binary data |
 | Custom binary | Small | Fast | Tightly controlled schemas |
 | MessagePack | Medium | Fast | Dynamic schemas, moderate size |
 | JSON | Large | Slow | Debugging, interop, human-readable |
 
 ### Minimize Encode/Decode Boundaries
 
 - Decode once at ingress, encode once at egress.
 - Avoid re-encoding/decoding in intermediate transform stages.
 - Use `as<type>` consistently; type mismatches force re-serialization.
 
 ## Memory Optimization
 
 ### Queue Sizing
 
 - Size queues based on expected throughput and consumer speed.
 - Too small: producers block frequently.
 - Too large: memory waste and increased latency.
 - Rule of thumb: `capacity = throughput * max_latency_tolerance`.
 
 | Scenario | Recommended Queue Size |
 |---|---|
 | Low-throughput signaling | 64–256 entries |
 | Moderate message passing | 1K–4K entries |
 | High-throughput streaming | 8K–64K entries |
 | Burst-tolerant buffering | 64K–256K entries |
 
 ### Shared Memory Sizing
 
 - Size shared memory regions to the actual data they hold.
 - Use `_KiB` and `_MiB` suffixes for readability.
 - Over-allocating wastes address space; under-allocating causes failures.
 
 | Data | Recommended Size |
 |---|---|
 | Subscription tables | 128 KiB |
 | Request maps | 64 KiB |
 | Work queues | 256 KiB |
 | Mailbox storage | 512 KiB |
 | Channel buffers | 64 KiB |
 
 ## Backend Optimization
 
 ### Emit Efficient Code
 
 - Use stack allocation instead of heap allocation where possible.
 - Avoid unnecessary copies: pass by reference, use move semantics.
 - Inline small helper functions.
 - Use target-language idioms for performance (e.g., `bytes.Buffer` in Go,
   `StringIO` in Python).
 
 ### Leverage Platform Features
 
 - Use `futex` instead of `semaphore` on Linux for low-contention locks.
 - Use `eventfd` instead of `pipe` for signaling.
 - Use `io_uring` for batched I/O operations.
 - Use `sendfile`/`splice` for zero-copy data transfer.
 
 ## Profiling IPC-L Programs
 
 ### Identify Bottlenecks
 
 1. **Lock contention**: High CPU usage with low throughput indicates
    excessive lock contention. Look for wide critical sections.
 2. **Serialization overhead**: High CPU in encode/decode stages.
    Consider a more efficient format or binary protocol.
 3. **Queue buildup**: Growing queue sizes indicate slow consumers.
    Add more consumers or increase queue capacity.
 4. **Signal storms**: Frequent `notify`/`wait` cycles indicate
    fine-grained synchronization. Consider batching.
 5. **Memory pressure**: Large shared memory regions with high churn.
    Use ring buffers or pre-allocated pools.
 
 ### Benchmarking
 
 For each protocol, measure:
 
 - **Throughput**: Messages per second at steady state.
 - **Latency**: End-to-end time for a single message (p50, p99, p999).
 - **CPU usage**: User and system CPU time per message.
 - **Memory usage**: Resident set size and shared memory footprint.
 - **Contention**: Time spent waiting on locks and signals.
 
 ## Optimization Checklist
 
 - [ ] Use the fastest primitive that meets your requirements.
 - [ ] Minimize lock scope: lock only what you mutate.
 - [ ] Use `eventfd` or `futex` for signaling on Linux.
 - [ ] Size queues and shared memory appropriately.
 - [ ] Batch operations when possible.
 - [ ] Use efficient serialization formats.
 - [ ] Decode once, encode once.
 - [ ] Avoid unnecessary copies in the backend.
 - [ ] Profile before optimizing: measure, don't guess.
 - [ ] Test optimizations under realistic load.
