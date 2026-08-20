 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_reqrep(const std::string& listen_addr, int port) {
   using namespace ipctk::dsl;
 
   auto rep_socket = socket("rep_socket") = tcp.listen(listen_addr + ":" + std::to_string(port));
   auto req_socket = socket("req_socket") = tcp.connect(listen_addr + ":" + std::to_string(port));
 
   auto request_map   = shared("request_map")   = shm.open("/requests", 64_KiB);
   auto request_lock  = mutex("request_lock")   = semaphore("/requests.lock", 1);
   auto request_ready = signal("request_ready") = eventfd();
 
   auto request_path =
     pipe("request_path") =
       recv(req_socket)
       >> encode(as<request>)
       >> send(rep_socket)
       >> lock(request_lock)
       >> insert(request_map)
       >> notify(request_ready)
       >> unlock(request_lock);
 
   auto reply_path =
     pipe("reply_path") =
       recv(rep_socket)
       >> decode(as<reply>)
       >> lock(request_lock)
       >> lookup(request_map)
       >> match_caller(request_map)
       >> remove(request_map)
       >> unlock(request_lock)
       >> send(req_socket);
 
   auto dispatch_path =
     pipe("dispatch_path") =
       wait(request_ready)
       >> lock(request_lock)
       >> dequeue(request_map)
       >> unlock(request_lock)
       >> decode(as<payload>)
       >> invoke("handler")
       >> encode(as<result>)
       >> send(req_socket);
 
   return std::make_tuple(rep_socket, req_socket, request_map, request_lock,
                          request_ready, request_path, reply_path, dispatch_path);
 }
 
 } // namespace ipctk::stdipc
