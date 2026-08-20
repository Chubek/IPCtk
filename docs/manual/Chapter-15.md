 # Chapter 15: Security Considerations
 
 IPC protocols operate at trust boundaries. This chapter describes
 security risks and mitigations for IPCtk-based systems.
 
 ## Threat Model
 
 IPCtk protocols connect processes that may have different trust levels:
 
 - **Same-user processes**: Processes running under the same UID.
   Generally trusted, but defense-in-depth is recommended.
 - **Different-user processes**: Processes running under different UIDs.
   Require explicit permission checks.
 - **Container/host boundary**: Processes in different containers or
   between container and host. Require namespace-aware isolation.
 - **Network boundary**: Processes on different hosts. Require
   encryption, authentication, and authorization.
 
 ## Attack Surface
 
 ### Shared Memory
 
 **Risk**: Any process with access to the shared memory path can read
 or write the data.
 
 **Mitigations**:
 
 - Use restrictive file permissions on shared memory objects:
   `shm.open("/myapp.subs", 128_KiB, 0600)`.
 - Use a dedicated directory with restricted permissions.
 - Validate all data read from shared memory before use.
 - Clear shared memory before reuse to prevent data leakage.
 - Use `mmap` with `MAP_PRIVATE` for read-only access.
 
 ### Named Semaphores
 
 **Risk**: Any process can open a named semaphore and manipulate it,
 potentially causing deadlocks or race conditions.
 
 **Mitigations**:
 
 - Use unique, unpredictable semaphore names.
 - Set restrictive permissions on semaphore objects.
 - Use `sem_open` with `O_EXCL` to prevent hijacking.
 - Consider using `futex` for same-process or parent-child scenarios.
 
 ### Unix Domain Sockets
 
 **Risk**: Any process with filesystem access to the socket path can
 connect and send messages.
 
 **Mitigations**:
 
 - Place sockets in a directory with restrictive permissions (`0700`).
 - Use `SO_PEERCRED` to verify the connecting process's UID/GID.
 - Use abstract socket namespace (Linux) to avoid filesystem access.
 - Authenticate the first message on each connection.
 
 ### TCP/UDP Sockets
 
 **Risk**: Network-accessible sockets are exposed to remote attackers.
 
 **Mitigations**:
 
 - Bind to `127.0.0.1` for local-only communication.
 - Use TLS for encrypted communication over untrusted networks.
 - Authenticate connections with tokens or certificates.
 - Rate-limit connections to prevent DoS.
 - Validate message sizes to prevent buffer overflows.
 - Use `SO_REUSEADDR` carefully to avoid port hijacking.
 
 ### Pipes and FIFOs
 
 **Risk**: Any process with filesystem access to the FIFO path can read
 or write to it.
 
 **Mitigations**:
 
 - Use restrictive file permissions on FIFO paths.
 - Use `pipe()` (unnamed) for parent-child communication.
 - Validate data read from FIFOs before use.
 
 ### Signals
 
 **Risk**: Any process with permission can send signals to your process,
 potentially causing interruption or termination.
 
 **Mitigations**:
 
 - Use real-time signals (`SIGRTMIN`+) for IPC, not standard signals.
 - Use `sigaction` with `SA_SIGINFO` to receive sender information.
 - Validate the sender's PID before processing the signal.
 - Use `signalfd` (Linux) for reliable signal handling.
 
 ## Data Validation
 
 ### Deserialization Safety
 
 - Always validate data after `decode` stages.
 - Check for buffer overflows, integer overflows, and type confusion.
 - Use `validate(schema)` stages to enforce schema constraints.
 - Never trust the size field in a message; verify against the actual
   received data length.
 
 ### Message Integrity
 
 - Consider adding checksums or HMACs to messages crossing trust
   boundaries.
 - Use sequence numbers to detect replay attacks.
 - Use timestamps to detect stale messages.
 
 ## Denial of Service
 
 ### Queue Exhaustion
 
 **Risk**: A malicious producer floods a queue, exhausting memory.
 
 **Mitigations**:
 
 - Set maximum queue sizes (`mpmc.ring(4096)`).
 - Use back-pressure (`flow_control_path`) to slow producers.
 - Drop messages when the queue is full (with logging).
 - Use `counter` to track and limit per-producer rates.
 
 ### Connection Exhaustion
 
 **Risk**: A malicious client opens many connections, exhausting file
 descriptors.
 
 **Mitigations**:
 
 - Set connection limits on listen sockets.
 - Use timeouts for idle connections.
 - Rate-limit new connections per source address.
 
 ### CPU Exhaustion
 
 **Risk**: A malicious client sends expensive-to-process messages.
 
 **Mitigations**:
 
 - Validate message complexity before processing.
 - Set timeouts for handler invocation.
 - Use worker pools to isolate expensive operations.
 
 ## Secure Configuration Checklist
 
 ### Resource Permissions
 
 - [ ] Shared memory: `0600` or `0660` permissions.
 - [ ] Semaphores: `0600` permissions, `O_EXCL` flag.
 - [ ] Unix sockets: `0700` directory, `0600` socket file.
 - [ ] FIFOs: `0600` permissions.
 - [ ] TCP sockets: bind to `127.0.0.1` when local-only.
 
 ### Network Security
 
 - [ ] Use TLS for inter-host communication.
 - [ ] Authenticate connections before accepting messages.
 - [ ] Validate message sizes before allocation.
 - [ ] Rate-limit connections and messages.
 
 ### Data Safety
 
 - [ ] Validate all deserialized data.
 - [ ] Use schema validation for typed messages.
 - [ ] Clear shared memory before reuse.
 - [ ] Add integrity checks for messages crossing trust boundaries.
 
 ### Operational Security
 
 - [ ] Run with least privilege (drop unnecessary capabilities).
 - [ ] Use seccomp filters to restrict syscalls.
 - [ ] Use namespaces for container isolation.
 - [ ] Log security-relevant events (connection attempts, validation failures).
 - [ ] Monitor queue depths and connection counts.
 
 ## Security in Generated Code
 
 When writing ITKD backends, ensure generated code follows security best
 practices:
 
 - **Bounds checking**: Validate buffer sizes before read/write.
 - **Error handling**: Don't leak sensitive information in error messages.
 - **Resource cleanup**: Close sockets, unmap memory, destroy semaphores
   on error paths.
 - **Input validation**: Validate all external inputs before use.
 - **No unsafe defaults**: Generated code should be secure by default;
   require explicit opt-in for unsafe operations.
 
 ## Reporting Vulnerabilities
 
 If you discover a security vulnerability in IPCtk or its standard library:
 
 1. Do not open a public issue.
 2. Contact the maintainer directly with a detailed description.
 3. Include steps to reproduce and affected versions.
 4. Allow reasonable time for a fix before disclosure.
