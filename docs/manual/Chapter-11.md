 # Chapter 11: Native C++ DSL Guide
 
 The native C++ DSL provides an embedded domain-specific language for
 constructing IPC-L protocol graphs directly in C++ code. It uses the
 `ipctk::dsl` namespace and produces the same IR as the syntax-based DSL.
 
 ## When to Use the Native DSL
 
 Use the native DSL when:
 
 - You need to dynamically construct protocols based on runtime
   configuration.
 - You want type-checking and IDE support for protocol construction.
 - You are integrating IPCtk into a larger C++ codebase.
 - You need to compose protocols programmatically.
 
 Use the syntax-based DSL (`.ipcl` files) when:
 
 - You want a declarative, language-agnostic protocol definition.
 - You need to share protocol definitions across teams or languages.
 - You prefer a concise, readable syntax for protocol graphs.
 - You are using the standard library protocols as-is.
 
 ## DSL Namespace
 
 ```cpp
 using namespace ipctk::dsl;
 ```
 
 The `ipctk::dsl` namespace provides builder functions for every IPC-L
 construct. Each builder returns an expression object that can be composed
 with `>>` (the pipe operator).
 
 ## Resource Builders
 
 ### Sockets
 
 ```cpp
 auto s = socket("name") = tcp.listen("127.0.0.1:7000");
 auto s = socket("name") = tcp.connect("127.0.0.1:8000");
 auto s = socket("name") = udp.bind("0.0.0.0:9000");
 auto s = socket("name") = udp.multicast("239.0.0.1:9000");
 auto s = socket("name") = unix.listen("/tmp/ipctk.sock");
 auto s = socket("name") = unix.connect("/tmp/ipctk.sock");
 ```
 
 ### Shared Memory
 
 ```cpp
 auto sh = shared("name") = shm.open("/path", 128_KiB);
 ```
 
 The `128_KiB` literal is provided by the DSL for readable sizes.
 
 ### Mutexes
 
 ```cpp
 auto m = mutex("name") = semaphore("/path", 1);
 auto m = mutex("name") = futex(0);
 ```
 
 ### Queues
 
 ```cpp
 auto q = queue("name") = mpmc.ring(4096);
 auto q = queue("name") = mqueue.open("/path", 10);
 ```
 
 ### Signals
 
 ```cpp
 auto s = signal("name") = eventfd();
 auto s = signal("name") = signal(SIGUSR1);
 ```
 
 ### Timers
 
 ```cpp
 auto t = timer("name") = timer(5000_ms);
 ```
 
 ### Counters
 
 ```cpp
 auto c = counter("name") = counter(0);
 ```
 
 ### Rings
 
 ```cpp
 auto r = ring("name") = io_uring.setup(256);
 ```
 
 ## Pipe Builder
 
 ```cpp
 auto p = pipe("name") =
   stage1
   >> stage2
   >> stage3;
 ```
 
 The `>>` operator chains stages left-to-right, matching the `->` syntax
 in IPC-L.
 
 ## Stage Builders
 
 ### Ingress Stages
 
 ```cpp
 recv(socket_resource)
 read(pipe_resource)
 wait(signal_resource)
 expire(timer_resource)
 accept(producer)
 ```
 
 ### Transform Stages
 
 ```cpp
 decode(as<message>)     // deserialize to typed payload
 encode(as<result>)      // serialize from typed payload
 map("function_name")    // apply a transformation
 filter(index_resource)  // filter by subscription
 route(table_resource)   // route to named channels
 validate(schema)        // validate against schema
 marshal(schema)         // marshal to schema format
 unmarshal(schema)       // unmarshal from schema format
 prioritize(queue)       // insert with priority
 frame(buffer)           // add framing
 deframe(buffer)         // remove framing
 ```
 
 ### State Stages
 
 ```cpp
 lock(mutex_resource)
 unlock(mutex_resource)
 insert(table_resource)
 update(table_resource)
 remove(table_resource)
 lookup(table_resource)
 deposit(mailbox_resource)
 retrieve(mailbox_resource)
 peek(mailbox_resource)
 clear(table_resource)
 ```
 
 ### Egress Stages
 
 ```cpp
 send(socket_resource)
 write(pipe_resource)
 notify(signal_resource)
 fanout(send(socket_resource))
 broadcast(udp.multicast("addr:port"))
 ```
 
 ### Dispatch Stages
 
 ```cpp
 enqueue(queue_resource)
 dequeue(queue_resource)
 invoke("handler_name")
 match_subscribers(table_resource)
 match_handlers(registry_resource)
 match_caller(call_table_resource)
 match_channels(channel_table_resource)
 assign_worker(counter_resource)
 next(counter_resource)
 aggregate(table_resource)
 drain(buffer_resource)
 start(timer_resource)
 ```
 
 ## Complete Example
 
 ```cpp
 #include <ipctk.hpp>
 
 using namespace ipctk::dsl;
 
 auto pub_in = socket("pub_in") = tcp.listen("127.0.0.1:7000");
 auto sub_in = socket("sub_in") = tcp.listen("127.0.0.1:7001");
 
 auto sub_table = shared("sub_table") = shm.open("/subs", 128_KiB);
 auto sub_lock  = mutex("sub_lock")   = semaphore("/subs.lock", 1);
 auto pub_queue = queue("pub_queue")  = mpmc.ring(4096);
 auto pub_ready = signal("pub_ready") = eventfd();
 
 auto publish_path =
   pipe("publish_path") =
     recv(pub_in)
     >> decode(as<message>)
     >> enqueue(pub_queue)
     >> notify(pub_ready);
 
 auto subscribe_path =
   pipe("subscribe_path") =
     recv(sub_in)
     >> decode(as<subscription>)
     >> lock(sub_lock)
     >> update(sub_table)
     >> unlock(sub_lock);
 
 auto dispatch_path =
   pipe("dispatch_path") =
     wait(pub_ready)
     >> dequeue(pub_queue)
     >> lock(sub_lock)
     >> match_subscribers(sub_table)
     >> unlock(sub_lock)
     >> fanout(send(sub_in));
 ```
 
 ## Type Safety
 
 The native DSL provides compile-time type checking:
 
 - `recv()` requires a socket resource.
 - `lock()` requires a mutex resource.
 - `decode()` and `encode()` use `as<T>` to specify the payload type.
 - Pipe chaining with `>>` checks stage compatibility.
 
 ## Interop with Syntax DSL
 
 Programs built with the native DSL and programs parsed from `.ipcl` files
 produce the same `ir::Program` IR. You can mix them:
 
 ```cpp
 // Build part of the program with the native DSL
 auto [pub_in, sub_in, ...] = ipctk::stdipc::build_pubsub("127.0.0.1", 7000, 7001);
 
 // Parse additional pipes from a file
 auto extra = ipctk::parse::parse_program_file("custom_pipes.ipcl");
 
 // Combine resources and pipes from both sources
 ```
 
 ## Best Practices
 
 - Use `using namespace ipctk::dsl` at function scope, not in headers.
 - Name all resources and pipes with descriptive identifiers.
 - Group resource declarations at the top of the function.
 - Use `auto` for all DSL expressions; the types are complex.
 - Keep pipe definitions on separate lines for readability.
 - Use `as<T>` consistently for typed decode/encode boundaries.
 - Ensure lock/unlock pairing is visible in the same function scope.
 - Wrap protocol construction in a function if it will be reused.
