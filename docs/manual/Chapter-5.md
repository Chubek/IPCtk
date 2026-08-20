 # Chapter 5: ITKD Language Reference
 
 ITKD (IPC-Toolkit Destination) is the backend mapping and generation
 language for IPCtk. It defines how IPC-L semantic constructs map to
 target-language code.
 
 ## File Structure
 
 An ITKD file consists of:
 
 1. **Target declaration**: The target language name.
 2. **Capability declarations**: The IPC-L capabilities this backend supports.
 3. **Import/include directives**: Target-language dependencies.
 4. **Type mappings**: IPC-L types → target-language types.
 5. **Emission rules**: Operation → code template mappings.
 
 Example (`dest/Python.itkd`):
 
 ```
 target python
 capability reqrep
 capability pubsub
 import asyncio
 import dataclasses
 map string -> str
 map bytes -> bytes
 rule emit_channel = template("async def send(channel, payload):\n    await channel.send(payload)\n")
 ```
 
 ## Target Declaration
 
 ```
 target <name>
 ```
 
 The target name identifies the language. It must be the first declaration
 in the file. Names are case-sensitive.
 
 | Target | File | Status |
 |---|---|---|
 | `c` | `dest/C.itkd` | Bundled |
 | `python` | `dest/Python.itkd` | Bundled |
 | `ruby` | `dest/Ruby.itkd` | Bundled |
 
 ## Capability Declarations
 
 ```
 capability <name>
 ```
 
 Capabilities declare which IPC-L protocol families the backend supports.
 A program using a protocol that requires a capability the backend does not
 declare will fail compatibility validation.
 
 | Capability | Protocols |
 |---|---|
 | `pubsub` | pubsub, messagebus, eventbus |
 | `reqrep` | reqrep, rpc |
 | `survey` | survey |
 | `pushpull` | pushpull |
 | `mailbox` | mailbox |
 | `channel` | channel |
 
 ## Import/Include Directives
 
 ```
 include <header>
 import <module>
 ```
 
 `include` is for C/C++ backends; `import` is for scripting languages.
 These directives are emitted at the top of the generated output.
 
 ## Type Mappings
 
 ```
 map <ipcl_type> -> <target_type>
 ```
 
 Type mappings define how IPC-L types translate to target-language types.
 
 Examples:
 
 ```
 map string -> const char*     # C
 map bytes  -> uint8_t*        # C
 map string -> str             # Python
 map bytes  -> bytes           # Python
 map string -> String          # Ruby
 map bytes  -> String          # Ruby
 ```
 
 ## Emission Rules
 
 ```
 rule <operation_name> = template("<code>")
 ```
 
 Each rule maps an IPC-L operation to a code template. The template is
 emitted when the compiler encounters that operation in a pipe.
 
 Rules can use placeholder substitution for:
 
 - Resource names: `${resource_name}`
 - Argument values: `${arg_N}`
 - Pipe names: `${pipe_name}`
 
 Example:
 
 ```
 rule emit_channel = template("int send_channel(int fd, const uint8_t* data, size_t len);")
 ```
 
 ## Built-in Operations
 
 Some operations are built into the compiler and do not require explicit
 rules. These include:
 
 - `recv`, `send` — socket I/O.
 - `lock`, `unlock` — mutex operations.
 - `encode`, `decode` — serialization boundaries.
 - `wait`, `notify` — signal synchronization.
 - `enqueue`, `dequeue` — queue operations.
 
 The compiler recognizes these and emits appropriate code based on the
 target language. Custom rules can override built-in behavior.
 
 ## Writing a Custom Backend
 
 To create a new backend:
 
 1. Create a `<target>.itkd` file.
 2. Declare the target name.
 3. List the capabilities you support.
 4. Add any necessary imports/includes.
 5. Define type mappings for IPC-L types.
 6. Add rules for each operation your backend handles.
 
 Example minimal backend for Go:
 
 ```
 target go
 capability pubsub
 import "net"
 import "sync"
 map string -> string
 map bytes  -> []byte
 rule send_channel = template("func sendChannel(ch net.Conn, data []byte) error {\n    _, err := ch.Write(data)\n    return err\n}\n")
 rule recv_channel = template("func recvChannel(ch net.Conn) ([]byte, error) {\n    buf := make([]byte, 4096)\n    n, err := ch.Read(buf)\n    return buf[:n], err\n}\n")
 ```
 
 ## Validation Rules for Backends
 
 1. **Target must be non-empty**: The target name is required.
 2. **Rules must have operation and emit**: Each rule needs both fields.
 3. **No duplicate operations**: Two rules cannot map the same operation.
 4. **Coverage check**: At compile time, every operation in the program
    must have a matching rule (built-in or custom).
 5. **Capability check**: The backend must declare all capabilities
    required by the program's operations.
 
 ## Constraints
 
 - **Deterministic symbol naming**: Generated identifiers must be
   predictable and repeatable across builds.
 - **Full coverage**: Every emitted semantic construct must have a
   corresponding rule.
 - **Explicit failures**: When a mapping gap exists, the compiler must
   produce a clear error, not silently skip the operation.
 - **Order preservation**: Stage ordering in pipes must be preserved in
   the generated output.
 - **No target logic in IPC-L**: IPC-L source files must not contain
   target-specific code or assumptions.
