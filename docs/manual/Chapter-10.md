 # Chapter 10: IPC Primitives Reference
 
 This chapter catalogs every IPC primitive available in IPCtk, organized
 by category. Each primitive is defined as a minimal IPC-L graph in
 `stdipc/primitives/` and can be composed into higher-level protocols.
 
 ## Stream Primitives
 
 ### pipe — Unidirectional Pipe
 
 A byte stream between parent and child processes. Data written to the
 write end is read from the read end in FIFO order.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/pipe.ipcl` |
 | Platform | All POSIX |
 | Resources | `pipe.rd()`, `pipe.wr()` |
 | Pipes | `pipe_read_path`, `pipe_write_path` |
 | Stages | `read`, `write`, `buffer`, `deliver`, `accept` |
 
 ### fifo — Named Pipe (FIFO)
 
 A named pipe for communication between unrelated processes. Created in
 the filesystem and opened by path.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/fifo.ipcl` |
 | Platform | All POSIX |
 | Resources | `fifo.open("/tmp/ipctk.fifo")` |
 | Pipes | `fifo_read_path`, `fifo_write_path` |
 | Stages | `open`, `read`, `write`, `deliver`, `accept` |
 
 ## Socket Primitives
 
 ### tcp_socket — TCP Stream Socket
 
 A reliable, ordered, connection-oriented byte stream over IP.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/tcp_socket.ipcl` |
 | Platform | All |
 | Resources | `tcp.listen(addr)`, `tcp.connect(addr)` |
 | Pipes | `tcp_accept_path`, `tcp_connect_path` |
 | Stages | `bind`, `listen`, `accept`, `connect`, `recv`, `send`, `close` |
 
 ### udp_socket — UDP Datagram Socket
 
 An unreliable, unordered, connectionless datagram service with optional
 multicast support.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/udp_socket.ipcl` |
 | Platform | All |
 | Resources | `udp.bind(addr)`, `udp.multicast(addr)` |
 | Pipes | `udp_send_path`, `udp_recv_path` |
 | Stages | `bind`, `sendto`, `recvfrom`, `encode`, `decode`, `close` |
 
 ### unix_socket — Unix Domain Socket
 
 A socket for communication between processes on the same host. Faster
 than TCP for local IPC.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/unix_socket.ipcl` |
 | Platform | All POSIX |
 | Resources | `unix.listen(path)`, `unix.connect(path)` |
 | Pipes | `unix_accept_path`, `unix_connect_path` |
 | Stages | `bind`, `listen`, `accept`, `connect`, `recv`, `send`, `close` |
 
 ## Shared Memory Primitives
 
 ### shm — POSIX Shared Memory
 
 A memory region shared between processes, optionally protected by a
 semaphore.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/shm.ipcl` |
 | Platform | All POSIX |
 | Resources | `shm.open(path, size)`, `semaphore(path, count)` |
 | Pipes | `shm_read_path`, `shm_write_path` |
 | Stages | `lock`, `unlock`, `map`, `unmap`, `copy`, `write` |
 
 ### mmap — Memory-Mapped File
 
 A file-backed shared memory region accessible by multiple processes.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/mmap.ipcl` |
 | Platform | All POSIX |
 | Resources | `mmap.open(path, size)` |
 | Pipes | `mmap_read_path`, `mmap_write_path` |
 | Stages | `map`, `unmap`, `copy`, `write`, `sync` |
 
 ## Synchronization Primitives
 
 ### semaphore — POSIX Semaphore
 
 A named semaphore for mutual exclusion and signaling between processes.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/semaphore.ipcl` |
 | Platform | All POSIX |
 | Resources | `semaphore(path, count)` |
 | Pipes | `sem_wait_path`, `sem_signal_path` |
 | Stages | `wait`, `post`, `enter`, `exit`, `notify` |
 
 ### futex — Fast Userspace Mutex
 
 A low-contention locking primitive that avoids kernel transitions when
 uncontended.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/futex.ipcl` |
 | Platform | Linux only |
 | Resources | `futex(initial)` |
 | Pipes | `futex_lock_path`, `futex_unlock_path` |
 | Stages | `cmp_xchg`, `futex_wait`, `futex_wake`, `enter`, `exit` |
 
 ### signal — OS Signal
 
 Asynchronous signal delivery between processes. Signals interrupt the
 receiving process and invoke a registered handler.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/signal.ipcl` |
 | Platform | All POSIX |
 | Resources | `signal(SIGUSR1)` |
 | Pipes | `sig_send_path`, `sig_recv_path` |
 | Stages | `target`, `send`, `register`, `wait`, `invoke`, `acknowledge` |
 
 ### eventfd — Event File Descriptor
 
 A file-descriptor-based event notification mechanism. Efficient for
 producer-consumer signaling.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/eventfd.ipcl` |
 | Platform | Linux only |
 | Resources | `eventfd(init, flags)` |
 | Pipes | `eventfd_notify_path`, `eventfd_wait_path` |
 | Stages | `write`, `read`, `wake`, `consume`, `close` |
 
 ## Queue Primitives
 
 ### mqueue — POSIX Message Queue
 
 A message queue with priority support. Messages are delivered in priority
 order within each priority band.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/mqueue.ipcl` |
 | Platform | All POSIX |
 | Resources | `mqueue.open(path, max_msgs)` |
 | Pipes | `mq_send_path`, `mq_recv_path` |
 | Stages | `open`, `send`, `receive`, `encode`, `decode`, `close` |
 
 ## Async I/O Primitives
 
 ### io_uring — Linux I/O Ring
 
 A high-performance async I/O submission and completion ring. Efficient
 for batched I/O operations.
 
 | Property | Value |
 |---|---|
 | File | `stdipc/primitives/io_uring.ipcl` |
 | Platform | Linux only (kernel ≥ 5.1) |
 | Resources | `io_uring.setup(entries)` |
 | Pipes | `uring_submit_path`, `uring_complete_path` |
 | Stages | `prepare`, `submit`, `enter`, `wait`, `peek`, `complete`, `advance` |
 
 ## Platform Availability Matrix
 
 | Primitive | Linux | macOS | FreeBSD | Windows |
 |---|---|---|---|---|
 | pipe | ✓ | ✓ | ✓ | — |
 | fifo | ✓ | ✓ | ✓ | — |
 | shm | ✓ | ✓ | ✓ | — |
 | mmap | ✓ | ✓ | ✓ | — |
 | mqueue | ✓ | — | — | — |
 | tcp_socket | ✓ | ✓ | ✓ | ✓ |
 | udp_socket | ✓ | ✓ | ✓ | ✓ |
 | unix_socket | ✓ | ✓ | ✓ | — |
 | semaphore | ✓ | ✓ | ✓ | — |
 | signal | ✓ | ✓ | ✓ | — |
 | futex | ✓ | — | — | — |
 | eventfd | ✓ | — | — | — |
 | io_uring | ✓ | — | — | — |
 
 ## Choosing a Primitive
 
 | Need | Recommended Primitive |
 |---|---|
 | Parent-child IPC | `pipe` |
 | Unrelated processes, local | `fifo` or `unix_socket` |
 | Network IPC | `tcp_socket` |
 | Broadcast/multicast | `udp_socket` |
 | Large shared data | `shm` or `mmap` |
 | Mutual exclusion | `semaphore` (portable) or `futex` (Linux, fast) |
 | Event notification | `eventfd` (Linux) or `semaphore` (portable) |
 | Priority messaging | `mqueue` |
 | Async batch I/O | `io_uring` (Linux) |
 | Out-of-band notification | `signal` |
