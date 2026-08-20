 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_pubsub(const std::string& listen_addr,
                           int pub_port,
                           int sub_port) {
   using namespace ipctk::dsl;
 
   auto pub_in = socket("pub_in") = tcp.listen(listen_addr + ":" + std::to_string(pub_port));
   auto sub_in = socket("sub_in") = tcp.listen(listen_addr + ":" + std::to_string(sub_port));
 
   auto subscription_table = shared("subscription_table") = shm.open("/subscriptions", 128_KiB);
   auto subscription_lock  = mutex("subscription_lock")   = semaphore("/subscriptions.lock", 1);
 
   auto publication_queue  = queue("publication_queue")   = mpmc.ring(4096);
   auto publication_ready  = signal("publication_ready")  = eventfd();
 
   auto publish_path =
     pipe("publish_path") =
       recv(pub_in)
       >> decode(as<message>)
       >> enqueue(publication_queue)
       >> notify(publication_ready);
 
   auto subscribe_path =
     pipe("subscribe_path") =
       recv(sub_in)
       >> decode(as<subscription>)
       >> lock(subscription_lock)
       >> update(subscription_table)
       >> unlock(subscription_lock);
 
   auto dispatch_path =
     pipe("dispatch_path") =
       wait(publication_ready)
       >> dequeue(publication_queue)
       >> lock(subscription_lock)
       >> match_subscribers(subscription_table)
       >> unlock(subscription_lock)
       >> fanout(send(sub_in));
 
   return std::make_tuple(pub_in, sub_in, subscription_table, subscription_lock,
                          publication_queue, publication_ready,
                          publish_path, subscribe_path, dispatch_path);
 }
 
 } // namespace ipctk::stdipc
