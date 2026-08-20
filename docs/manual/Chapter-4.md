 # Chapter 4: IPC-L Language Reference
 
 IPC-L is the protocol/flow definition language for IPCtk. It exists in
 two forms: a syntax-based DSL parsed from `.ipcl` files, and a C++ native
 DSL using the `ipctk::dsl` namespace. Both produce the same IR.
 
 ## Lexical Conventions
 
 - **Source encoding**: UTF-8.
 - **Comments**: `//` to end of line, `/* ... */` for block comments.
 - **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_.-]*`
 - **Keywords**: `socket`, `shared`, `mutex`, `queue`, `signal`, `pipe`,
   `timer`, `counter`, `fifo`, `ring`.
 - **Separators**: `;` terminates declarations, `->` separates pipe stages.
 - **Literals**: string literals in double quotes, numeric sizes with
   optional suffix (`_KiB`, `_MiB`, `_ms`).
 
 ## Resource Declarations
 
 Resources are declared at the top of an IPC-L file, before any pipe
 definitions. Each resource has a kind, a name, and an initializer.
 
 ### Socket
 
 ```
 socket <name> = <transport>.<method>("<address>");
 ```
 
 Transports:
 
 | Transport | Methods | Description |
 |---|---|---|
 | `tcp` | `listen`, `connect` | TCP stream sockets |
 | `udp` | `bind`, `multicast` | UDP datagram sockets |
 | `unix` | `listen`, `connect` | Unix domain sockets |
 
 Examples:
 
 ```
 socket pub_in  = tcp.listen("127.0.0.1:7000");
 socket req_sock = tcp.connect("127.0.0.1:8000");
 socket survey   = udp.multicast("239.0.0.1:9000");
 socket channel  = unix.listen("/tmp/ipctk.sock");
 ```
 
 ### Shared Memory
 
 ```
 shared <name> = shm.open("<path>", <size>);
 ```
 
 Sizes can use `_KiB` or `_MiB` suffixes:
 
 ```
 shared sub_table = shm.open("/subscriptions", 128_KiB);
 shared work_buf  = shm.open("/work", 4_KiB);
 ```
 
 ### Mutex / Semaphore
 
 ```
 mutex <name> = semaphore("<path>", <count>);
 ```
 
 The count is the initial semaphore value (typically 1 for a mutex):
 
 ```
 mutex sub_lock = semaphore("/subscriptions.lock", 1);
 ```
 
 ### Queue
 
 ```
 queue <name> = <type>.<method>(<capacity>);
 ```
 
 Queue types:
 
 | Type | Method | Description |
 |---|---|---|
 | `mpmc` | `ring` | Multi-producer, multi-consumer ring buffer |
 | `mqueue` | `open` | POSIX message queue |
 
 ```
 queue pub_queue = mpmc.ring(4096);
 queue msg_queue = mqueue.open("/ipctk.mq", 10);
 ```
 
 ### Signal / Event
 
 ```
 signal <name> = <method>();
 ```
 
 | Method | Platform | Description |
 |---|---|---|
 | `eventfd` | Linux | File-descriptor event notification |
 | `signal(SIGUSR1)` | POSIX | OS signal delivery |
 
 ```
 signal pub_ready = eventfd();
 signal sig       = signal(SIGUSR1);
 ```
 
 ### Timer
 
 ```
 timer <name> = timer(<duration>);
 ```
 
 Durations use `_ms` suffix:
 
 ```
 timer deadline = timer(5000_ms);
 ```
 
 ### Counter
 
 ```
 counter <name> = counter(<initial>);
 ```
 
 ```
 counter rr = counter(0);
 ```
 
 ### Mutex (Futex)
 
 ```
 mutex <name> = futex(<initial>);
 ```
 
 Linux only:
 
 ```
 mutex ftx = futex(0);
 ```
 
 ### Ring (io_uring)
 
 ```
 ring <name> = io_uring.setup(<entries>);
 ```
 
 Linux only:
 
 ```
 ring io_ring = io_uring.setup(256);
 ```
 
 ## Pipe Definitions
 
 A pipe is a named sequence of stages connected by `->`:
 
 ```
 pipe <name> =
     <stage1>
     -> <stage2>
     -> <stage3>;
 ```
 
 ## Stage Reference
 
 ### Ingress Stages
 
 | Stage | Arguments | Description |
 |---|---|---|
 | `recv(socket)` | socket resource | Receive data from a socket |
 | `read(pipe)` | pipe resource | Read from a pipe/FIFO |
 | `wait(signal)` | signal resource | Block until signal is ready |
 | `expire(timer)` | timer resource | Trigger on timer expiry |
 | `accept(producer)` | producer reference | Accept data from a producer |
 
 ### Transform Stages
 
 | Stage | Arguments | Description |
 |---|---|---|
 | `decode(type)` | type name | Deserialize incoming bytes to a typed payload |
 | `encode(type)` | type name | Serialize a typed payload to bytes |
 | `map(fn)` | function name | Apply a transformation function |
 | `filter(index)` | index resource | Filter messages by subscription criteria |
 | `route(table)` | channel table | Route messages to named channels |
 | `validate(schema)` | schema resource | Validate payload against a schema |
 | `marshal(schema)` | schema resource | Marshal a value to schema format |
 | `unmarshal(schema)` | schema resource | Unmarshal from schema format |
 | `prioritize(queue)` | queue resource | Insert with priority ordering |
 | `frame(buf)` | buffer resource | Add framing to a message |
 | `deframe(buf)` | buffer resource | Remove framing from a message |
 
 ### State Stages
 
 | Stage | Arguments | Description |
 |---|---|---|
 | `lock(mutex)` | mutex resource | Acquire a mutex |
 | `unlock(mutex)` | mutex resource | Release a mutex |
 | `insert(table)` | table resource | Insert an entry into a shared table |
 | `update(table)` | table resource | Update an existing entry |
 | `remove(table)` | table resource | Remove an entry |
 | `lookup(table)` | table resource | Look up an entry by key |
 | `deposit(mailbox)` | mailbox resource | Deposit a message in a mailbox |
 | `retrieve(mailbox)` | mailbox resource | Retrieve a message from a mailbox |
 | `peek(mailbox)` | mailbox resource | Check mailbox without removing |
 | `clear(table)` | table resource | Clear all entries from a table |
 
 ### Egress Stages
 
 | Stage | Arguments | Description |
 |---|---|---|
 | `send(socket)` | socket resource | Send data to a socket |
 | `write(pipe)` | pipe resource | Write to a pipe/FIFO |
 | `notify(signal)` | signal resource | Signal readiness |
 | `fanout(send(…))` | nested send | Fan out to multiple receivers |
 | `broadcast(addr)` | multicast address | Broadcast to multiple receivers |
 
 ### Dispatch Stages
 
 | Stage | Arguments | Description |
 |---|---|---|
 | `enqueue(queue)` | queue resource | Enqueue an item |
 | `dequeue(queue)` | queue resource | Dequeue an item |
 | `invoke(handler)` | handler name | Invoke a handler function |
 | `match_subscribers(table)` | subscription table | Match subscribers to a topic |
 | `match_handlers(registry)` | handler registry | Match handlers to an event type |
 | `match_caller(table)` | call table | Match a reply to its caller |
 | `match_channels(table)` | channel table | Match channels for routing |
 | `assign_worker(counter)` | counter resource | Assign to a worker via round-robin |
 | `next(counter)` | counter resource | Advance the round-robin counter |
 | `aggregate(table)` | table resource | Aggregate results from a table |
 | `drain(buf)` | buffer resource | Drain a buffer for flow control |
 | `start(timer)` | timer resource | Start a timer |
 
 ## Validation Rules
 
 1. **Declaration before use**: All resources referenced in pipes must be
    declared before the pipe that uses them.
 2. **Unique pipe names**: Pipe names must be unique within a program.
 3. **Non-empty pipes**: Every pipe must have at least one stage.
 4. **Stage compatibility**: The output type of one stage must be compatible
    with the input type of the next stage in the chain.
 5. **Lock/unlock pairing**: Every `lock` must be paired with a corresponding
    `unlock` on the same mutex within the same pipe or a coordinated pipe set.
 6. **Resource kind matching**: Operations must use resources of the correct
    kind (e.g., `recv` requires a socket, `lock` requires a mutex).
 7. **No unresolved terminals**: Pipes must not end on an intermediate
    transform stage without an egress stage.
 
 ## Common Failures
 
 | Failure | Cause | Fix |
 |---|---|---|
 | Unresolved identifier | Resource used before declaration | Move declaration above the pipe |
 | Duplicate pipe name | Two pipes share a name | Rename one of them |
 | Empty pipe | Pipe has no stages | Add at least one stage |
 | Incompatible stage chain | Type mismatch between stages | Insert a `decode` or `encode` stage |
 | Missing ITKD coverage | No backend rule for an operation | Add a rule to the backend `.itkd` file |
 | Unpaired lock/unlock | Lock without matching unlock | Add the missing unlock stage |
 | Resource kind mismatch | Wrong resource type for operation | Use a resource of the correct kind |
