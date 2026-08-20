 # Chapter 13: Backend Authoring
 
 This chapter is a guide for writing custom ITKD backends that target
 new languages or runtime environments.
 
 ## Backend Architecture
 
 A backend is an `ir::BackendSpec` that describes:
 
 1. The target language or runtime.
 2. The capabilities it supports.
 3. Type mappings from IPC-L types to target types.
 4. Emission rules for each IPC-L operation.
 
 ## Step-by-Step: Creating a Backend
 
 ### Step 1: Choose a Target Name
 
 The target name identifies your backend. It should be lowercase and
 descriptive:
 
 ```
 target go
 target rust
 target javascript
 target java
 ```
 
 ### Step 2: Declare Capabilities
 
 List the IPC-L protocol families your backend supports:
 
 ```
 capability pubsub
 capability reqrep
 ```
 
 If your backend supports all protocols, declare all capabilities:
 
 ```
 capability pubsub
 capability reqrep
 capability survey
 capability pushpull
 capability mailbox
 capability channel
 ```
 
 ### Step 3: Add Imports/Includes
 
 Declare the target-language dependencies your generated code needs:
 
 ```
 import "net"
 import "sync"
 import "encoding/json"
 ```
 
 For C/C++ backends, use `include`:
 
 ```
 include <stdint.h>
 include <stddef.h>
 include <sys/socket.h>
 ```
 
 ### Step 4: Define Type Mappings
 
 Map IPC-L types to your target language's types:
 
 ```
 map string -> string
 map bytes  -> []byte
 map int    -> int64
 map float  -> float64
 ```
 
 ### Step 5: Write Emission Rules
 
 Each rule maps an IPC-L operation to target code. Use the `template()`
 function with the target code as a string literal.
 
 ```
 rule send_channel = template("
 func sendChannel(ch net.Conn, data []byte) error {
     _, err := ch.Write(data)
     return err
 }
 ")
 
 rule recv_channel = template("
 func recvChannel(ch net.Conn) ([]byte, error) {
     buf := make([]byte, 4096)
     n, err := ch.Read(buf)
     if err != nil {
         return nil, err
     }
     return buf[:n], nil
 }
 ")
 ```
 
 ### Step 6: Save and Validate
 
 Save your backend as `<target>.itkd` in the `dest/` directory or your
 project's own directory.
 
 Validate it:
 
 ```cpp
 #include <ipctk.hpp>
 
 auto spec = ipctk::parse::parse_backend_file("dest/go.itkd");
 if (spec.is_err()) {
   std::cerr << "Backend parse error: " << spec.unwrap_err().message << "\n";
   return 1;
 }
 auto valid = ipctk::validate::validate(spec.unwrap());
 if (valid.is_err()) {
   std::cerr << "Backend validation error: " << valid.unwrap_err().message << "\n";
   return 1;
 }
 std::cout << "Backend is valid.\n";
 ```
 
 ### Step 7: Test with a Known Program
 
 ```cpp
 auto program = ipctk::parse::parse_program_file("stdipc/protocols/pubsub.ipcl");
 auto backend = ipctk::parse::parse_backend_file("dest/go.itkd");
 auto result  = ipctk::compile(program.unwrap(), backend.unwrap());
 if (result.is_err()) {
   std::cerr << "Compilation error: " << result.unwrap_err().message << "\n";
   return 1;
 }
 std::cout << result.unwrap() << "\n";
 ```
 
 ## Complete Example: Go Backend
 
 ```
 target go
 
 capability pubsub
 capability reqrep
 
 import "net"
 import "sync"
 import "encoding/json"
 
 map string -> string
 map bytes  -> []byte
 map int    -> int64
 
 rule send_channel = template("
 func sendChannel(ch net.Conn, data []byte) error {
     _, err := ch.Write(data)
     return err
 }
 ")
 
 rule recv_channel = template("
 func recvChannel(ch net.Conn) ([]byte, error) {
     buf := make([]byte, 4096)
     n, err := ch.Read(buf)
     if err != nil {
         return nil, err
     }
     return buf[:n], nil
 }
 ")
 
 rule lock_mutex = template("
 func lockMutex(mu *sync.Mutex) {
     mu.Lock()
 }
 ")
 
 rule unlock_mutex = template("
 func unlockMutex(mu *sync.Mutex) {
     mu.Unlock()
 }
 ")
 
 rule encode_json = template("
 func encodeJSON(v interface{}) ([]byte, error) {
     return json.Marshal(v)
 }
 ")
 
 rule decode_json = template("
 func decodeJSON(data []byte, v interface{}) error {
     return json.Unmarshal(data, v)
 }
 ")
 ```
 
 ## Built-in Operations
 
 The compiler recognizes these operations as built-in and does not require
 explicit rules:
 
 - `recv`, `send` — socket I/O.
 - `lock`, `unlock` — mutex operations.
 - `encode`, `decode` — serialization boundaries.
 - `wait`, `notify` — signal synchronization.
 - `enqueue`, `dequeue` — queue operations.
 
 You can override built-in behavior by providing a rule with the same
 operation name. Your rule takes precedence.
 
 ## Emitted Output Structure
 
 The compiler produces output in this order:
 
 1. Target metadata (`# target <name>`).
 2. Capability declarations (`# capability <name>`).
 3. Polyflow scheduling status (`# scheduling polyflow=...`).
 4. Per-pipe emitted code, in declaration order.
 
 Each pipe section starts with `# pipe <name>` and contains the emitted
 code for each step in the pipe.
 
 ## Testing Your Backend
 
 ### Compatibility Test
 
 Test that your backend is compatible with all standard library protocols
 that declare the capabilities you support:
 
 ```cpp
 for (auto proto : {"pubsub", "reqrep", "messagebus", "eventbus", "rpc"}) {
   auto program = ipctk::parse::parse_program_file(
     "stdipc/protocols/" + std::string(proto) + ".ipcl");
   auto backend = ipctk::parse::parse_backend_file("dest/my_backend.itkd");
   auto result  = ipctk::compile(program.unwrap(), backend.unwrap());
   if (result.is_err()) {
     std::cerr << proto << " failed: " << result.unwrap_err().message << "\n";
   }
 }
 ```
 
 ### Round-Trip Test
 
 Verify that compiling and re-parsing the output produces the same IR:
 
 ```cpp
 auto program = ipctk::parse::parse_program_file("stdipc/protocols/pubsub.ipcl");
 auto backend = ipctk::parse::parse_backend_file("dest/my_backend.itkd");
 auto output  = ipctk::compile(program.unwrap(), backend.unwrap());
 // The output should be valid target-language code
 ```
 
 ## Backend Distribution
 
 ### Bundled with IPCtk
 
 Place your `.itkd` file in `dest/` and it will be installed with IPCtk.
 Update `CMakeLists.txt` to install it:
 
 ```cmake
 install(FILES dest/Go.itkd DESTINATION ${CMAKE_INSTALL_DATADIR}/stdipc/dest)
 ```
 
 ### Standalone
 
 Backends can be distributed independently. Users load them by path:
 
 ```cpp
 auto backend = ipctk::parse::parse_backend_file("/path/to/go.itkd");
 ```
 
 ## Best Practices
 
 - **Cover all operations**: Ensure every IPC-L operation used by your
   target protocols has a rule.
 - **Use target-language idioms**: Emit code that follows the target
   language's conventions.
 - **Handle errors**: Generated code should include error handling for
   I/O, memory, and synchronization failures.
 - **Document assumptions**: Note any runtime dependencies or environment
   requirements in comments.
 - **Version your backend**: Include a version comment in the `.itkd` file.
 - **Test with all protocols**: Run compatibility tests against every
   protocol that uses your backend's capabilities.
 - **Keep rules small**: Each rule should emit a single function or
   code block. Compose complex behavior from multiple rules.
