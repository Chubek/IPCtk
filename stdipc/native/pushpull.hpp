 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_pushpull(const std::string& listen_addr,
                             int push_port,
                             int pull_port) {
   using namespace ipctk::dsl;
 
   auto push_socket = socket("push_socket") = tcp.listen(listen_addr + ":" + std::to_string(push_port));
   auto pull_socket = socket("pull_socket") = tcp.listen(listen_addr + ":" + std::to_string(pull_port));
 
   auto work_queue   = shared("work_queue")   = shm.open("/work", 256_KiB);
   auto work_lock    = mutex("work_lock")     = semaphore("/work.lock", 1);
   auto work_ready   = signal("work_ready")   = eventfd();
   auto round_robin  = counter("round_robin") = counter(0);
 
   auto push_path =
     pipe("push_path") =
       recv(push_socket)
       >> decode(as<task>)
       >> lock(work_lock)
       >> enqueue(work_queue)
       >> notify(work_ready)
       >> unlock(work_lock);
 
   auto pull_path =
     pipe("pull_path") =
       recv(pull_socket)
       >> decode(as<ack>)
       >> send(pull_socket);
 
   auto distribute_path =
     pipe("distribute_path") =
       wait(work_ready)
       >> lock(work_lock)
       >> dequeue(work_queue)
       >> unlock(work_lock)
       >> next(round_robin)
       >> assign_worker(round_robin)
       >> send(pull_socket);
 
   return std::make_tuple(push_socket, pull_socket, work_queue, work_lock,
                          work_ready, round_robin, push_path, pull_path, distribute_path);
 }
 
 } // namespace ipctk::stdipc
