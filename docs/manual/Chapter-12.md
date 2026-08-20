 # Chapter 12: Protocol Design Patterns
 
 This chapter describes common patterns for composing IPC-L primitives
 into robust, maintainable protocols.
 
 ## Pattern 1: Fan-Out Dispatch
 
 **Problem**: Distribute messages from one producer to multiple consumers
 based on routing criteria.
 
 **Solution**: Use a shared subscription table, a dispatch pipe, and
 `fanout(send(...))`.
 
 ```
 socket ingress = tcp.listen("127.0.0.1:7000");
 shared route_table = shm.open("/routes", 128_KiB);
 mutex  route_lock  = semaphore("/routes.lock", 1);
 queue  msg_buffer  = mpmc.ring(4096);
 signal msg_ready   = eventfd();
 
 pipe ingress_path =
     recv(ingress)
     -> decode(message)
     -> enqueue(msg_buffer)
     -> notify(msg_ready);
 
 pipe dispatch_path =
     wait(msg_ready)
     -> dequeue(msg_buffer)
     -> lock(route_lock)
     -> match_subscribers(route_table)
     -> unlock(route_lock)
     -> fanout(send(ingress));
 ```
 
 **When to use**: Pub/Sub, event buses, message buses, any scenario where
 messages are routed to multiple interested parties.
 
 ## Pattern 2: Request-Response with Correlation
 
 **Problem**: Match responses to their originating requests in a
 multiplexed connection.
 
 **Solution**: Use a shared request map keyed by request ID, with
 `insert`, `lookup`, `match_caller`, and `remove` stages.
 
 ```
 socket server = tcp.listen("127.0.0.1:8000");
 socket client = tcp.connect("127.0.0.1:8000");
 shared request_map  = shm.open("/reqs", 64_KiB);
 mutex  request_lock = semaphore("/reqs.lock", 1);
 signal request_ready = eventfd();
 
 pipe request_path =
     recv(client)
     -> encode(request)
     -> send(server)
     -> lock(request_lock)
     -> insert(request_map)
     -> notify(request_ready)
     -> unlock(request_lock);
 
 pipe reply_path =
     recv(server)
     -> decode(reply)
     -> lock(request_lock)
     -> lookup(request_map)
     -> match_caller(request_map)
     -> remove(request_map)
     -> unlock(request_lock)
     -> send(client);
 ```
 
 **When to use**: RPC, Req/Rep, any synchronous request-response protocol.
 
 ## Pattern 3: Work Distribution with Round-Robin
 
 **Problem**: Distribute work items evenly across a pool of workers.
 
 **Solution**: Use a counter for round-robin assignment and
 `assign_worker` for dispatching.
 
 ```
 socket push_sock = tcp.listen("127.0.0.1:10000");
 socket pull_sock = tcp.listen("127.0.0.1:10001");
 shared work_queue = shm.open("/work", 256_KiB);
 mutex  work_lock  = semaphore("/work.lock", 1);
 signal work_ready = eventfd();
 counter rr        = counter(0);
 
 pipe distribute_path =
     wait(work_ready)
     -> lock(work_lock)
     -> dequeue(work_queue)
     -> unlock(work_lock)
     -> next(rr)
     -> assign_worker(rr)
     -> send(pull_sock);
 ```
 
 **When to use**: Push/Pull, worker pools, load-balanced task distribution.
 
 ## Pattern 4: Scatter-Gather with Deadline
 
 **Problem**: Broadcast a question to multiple respondents, collect
 answers within a time limit, and aggregate results.
 
 **Solution**: Use a UDP multicast for the broadcast, a shared table for
 responses, and a timer for the deadline.
 
 ```
 socket survey_out  = udp.multicast("239.0.0.1:9000");
 socket response_in = tcp.listen("127.0.0.1:9001");
 shared resp_table = shm.open("/resp", 128_KiB);
 mutex  resp_lock  = semaphore("/resp.lock", 1);
 timer  deadline   = timer(5000_ms);
 
 pipe collect_path =
     expire(deadline)
     -> lock(resp_lock)
     -> aggregate(resp_table)
     -> clear(resp_table)
     -> unlock(resp_lock)
     -> encode(results)
     -> send(survey_out);
 ```
 
 **When to use**: Surveyor/Respondent, distributed queries, health checks.
 
 ## Pattern 5: Priority Dispatch
 
 **Problem**: Process high-priority messages before low-priority ones.
 
 **Solution**: Use `prioritize` to insert messages into a priority queue
 and `dequeue` to retrieve them in priority order.
 
 ```
 queue priority_queue = mpmc.ring(4096);
 
 pipe priority_dispatch_path =
     wait(event_ready)
     -> dequeue(priority_queue)
     -> lock(handler_lock)
     -> match_handlers(handler_registry)
     -> unlock(handler_lock)
     -> fanout(send(event_socket));
 ```
 
 **When to use**: Event buses with priority, QoS-aware messaging,
   critical alerts.
 
 ## Pattern 6: Schema Registry
 
 **Problem**: Validate and serialize/deserialize messages with evolving
 schemas.
 
 **Solution**: Use a shared schema registry and `validate`, `marshal`,
 `unmarshal` stages.
 
 ```
 shared schema_registry = shm.open("/schemas", 128_KiB);
 
 pipe call_path =
     recv(rpc_socket)
     -> decode(call)
     -> lock(rpc_lock)
     -> validate(schema_registry)
     -> insert(call_table)
     -> enqueue(worker_queue)
     -> notify(completion_sig)
     -> unlock(rpc_lock);
 
 pipe invoke_path =
     wait(completion_sig)
     -> dequeue(worker_queue)
     -> lock(rpc_lock)
     -> lookup(call_table)
     -> unmarshal(schema_registry)
     -> invoke(procedure)
     -> marshal(schema_registry)
     -> update(call_table)
     -> unlock(rpc_lock)
     -> send(rpc_socket);
 ```
 
 **When to use**: RPC, data pipelines with schema evolution, typed APIs.
 
 ## Pattern 7: Back-Pressure Flow Control
 
 **Problem**: Prevent a fast producer from overwhelming a slow consumer.
 
 **Solution**: Use a shared buffer window and a back-pressure signal.
 When the buffer fills, the producer waits.
 
 ```
 signal backpressure  = eventfd();
 shared buffer_window = shm.open("/buf", 64_KiB);
 mutex  channel_lock  = semaphore("/chan.lock", 1);
 
 pipe send_path =
     recv(channel_socket)
     -> encode(payload)
     -> lock(channel_lock)
     -> frame(buffer_window)
     -> notify(backpressure)
     -> unlock(channel_lock)
     -> send(channel_socket);
 
 pipe flow_control_path =
     wait(backpressure)
     -> lock(channel_lock)
     -> drain(buffer_window)
     -> unlock(channel_lock)
     -> encode(ack)
     -> send(channel_socket);
 ```
 
 **When to use**: Channels, streaming protocols, rate-limited producers.
 
 ## Pattern 8: Asynchronous Mailbox
 
 **Problem**: Enable fire-and-forget message delivery with optional
 blocking receive.
 
 **Solution**: Use a shared mailbox storage, a per-mailbox lock, and
 separate `send`, `receive`, and `poll` pipes.
 
 ```
 shared mailbox_storage = shm.open("/mbox", 512_KiB);
 mutex  mailbox_lock    = semaphore("/mbox.lock", 1);
 signal arrival_signal  = eventfd();
 
 pipe send_path =
     recv(mailbox_socket)
     -> decode(message)
     -> lock(mailbox_lock)
     -> deposit(mailbox_storage)
     -> notify(arrival_signal)
     -> unlock(mailbox_lock);
 
 pipe receive_path =
     wait(arrival_signal)
     -> lock(mailbox_lock)
     -> retrieve(mailbox_storage)
     -> remove(mailbox_storage)
     -> unlock(mailbox_lock)
     -> decode(payload)
     -> send(mailbox_socket);
 
 pipe poll_path =
     recv(mailbox_socket)
     -> decode(poll_request)
     -> lock(mailbox_lock)
     -> peek(mailbox_storage)
     -> unlock(mailbox_lock)
     -> encode(count)
     -> send(mailbox_socket);
 ```
 
 **When to use**: Mailboxes, actor-model messaging, job queues.
 
 ## Pattern Selection Guide
 
 | Requirement | Pattern |
 |---|---|
 | One-to-many messaging | Fan-Out Dispatch |
 | Synchronous request-response | Request-Response with Correlation |
 | Load-balanced task distribution | Work Distribution with Round-Robin |
 | Broadcast with deadline | Scatter-Gather with Deadline |
 | QoS-aware messaging | Priority Dispatch |
 | Schema evolution | Schema Registry |
 | Rate limiting | Back-Pressure Flow Control |
 | Fire-and-forget messaging | Asynchronous Mailbox |
 
 ## Anti-Patterns
 
 - **Missing lock/unlock pairing**: Always pair `lock` and `unlock` on
   the same mutex within the same or coordinated pipe.
 - **Unbounded queues**: Always specify a capacity for queues; unbounded
   queues can exhaust memory.
 - **Blocking in egress stages**: Egress stages should be non-blocking;
   use signals and wait stages for synchronization.
 - **Mixing transports in one pipe**: Keep transport-specific stages
   (TCP, UDP, Unix) in separate pipes for clarity.
 - **Implicit ordering assumptions**: Do not assume ordering between
   pipes unless explicitly synchronized via signals or mutexes.
 - **Hardcoded addresses**: Use resource declarations with configurable
   addresses rather than hardcoding them in stage arguments.
