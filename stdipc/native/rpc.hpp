 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_rpc(const std::string& listen_addr, int port) {
   using namespace ipctk::dsl;
 
   auto rpc_socket = socket("rpc_socket") = tcp.listen(listen_addr + ":" + std::to_string(port));
 
   auto schema_registry = shared("schema_registry") = shm.open("/schemas", 128_KiB);
   auto call_table      = shared("call_table")      = shm.open("/calls", 256_KiB);
   auto rpc_lock        = mutex("rpc_lock")         = semaphore("/rpc.lock", 1);
   auto worker_queue    = queue("worker_queue")     = mpmc.ring(4096);
   auto completion_sig  = signal("completion_sig")  = eventfd();
 
   auto call_path =
     pipe("call_path") =
       recv(rpc_socket)
       >> decode(as<call>)
       >> lock(rpc_lock)
       >> validate(schema_registry)
       >> insert(call_table)
       >> enqueue(worker_queue)
       >> notify(completion_sig)
       >> unlock(rpc_lock);
 
   auto invoke_path =
     pipe("invoke_path") =
       wait(completion_sig)
       >> dequeue(worker_queue)
       >> lock(rpc_lock)
       >> lookup(call_table)
       >> unmarshal(schema_registry)
       >> invoke("procedure")
       >> marshal(schema_registry)
       >> update(call_table)
       >> unlock(rpc_lock)
       >> send(rpc_socket);
 
   auto return_path =
     pipe("return_path") =
       recv(rpc_socket)
       >> decode(as<result>)
       >> lock(rpc_lock)
       >> match_caller(call_table)
       >> remove(call_table)
       >> unlock(rpc_lock)
       >> send(rpc_socket);
 
   return std::make_tuple(rpc_socket, schema_registry, call_table, rpc_lock,
                          worker_queue, completion_sig,
                          call_path, invoke_path, return_path);
 }
 
 } // namespace ipctk::stdipc
