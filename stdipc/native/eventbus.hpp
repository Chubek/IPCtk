 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_eventbus(const std::string& listen_addr, int port) {
   using namespace ipctk::dsl;
 
   auto event_socket = socket("event_socket") = tcp.listen(listen_addr + ":" + std::to_string(port));
 
   auto handler_registry = shared("handler_registry") = shm.open("/handlers", 128_KiB);
   auto event_lock       = mutex("event_lock")        = semaphore("/event.lock", 1);
   auto priority_queue   = queue("priority_queue")    = mpmc.ring(4096);
   auto event_ready      = signal("event_ready")      = eventfd();
 
   auto emit_path =
     pipe("emit_path") =
       recv(event_socket)
       >> decode(as<event>)
       >> lock(event_lock)
       >> prioritize(priority_queue)
       >> notify(event_ready)
       >> unlock(event_lock);
 
   auto register_path =
     pipe("register_path") =
       recv(event_socket)
       >> decode(as<handler>)
       >> lock(event_lock)
       >> insert(handler_registry)
       >> unlock(event_lock);
 
   auto dispatch_path =
     pipe("dispatch_path") =
       wait(event_ready)
       >> dequeue(priority_queue)
       >> lock(event_lock)
       >> match_handlers(handler_registry)
       >> unlock(event_lock)
       >> fanout(send(event_socket));
 
   return std::make_tuple(event_socket, handler_registry, event_lock, priority_queue,
                          event_ready, emit_path, register_path, dispatch_path);
 }
 
 } // namespace ipctk::stdipc
