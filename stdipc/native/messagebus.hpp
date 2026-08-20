 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_messagebus(const std::string& listen_addr, int port) {
   using namespace ipctk::dsl;
 
   auto bus_socket = socket("bus_socket") = tcp.listen(listen_addr + ":" + std::to_string(port));
 
   auto channel_table    = shared("channel_table")    = shm.open("/channels", 256_KiB);
   auto subscription_idx = shared("subscription_idx") = shm.open("/sub_idx", 128_KiB);
   auto bus_lock         = mutex("bus_lock")          = semaphore("/bus.lock", 1);
   auto message_buffer   = queue("message_buffer")    = mpmc.ring(8192);
   auto message_ready    = signal("message_ready")    = eventfd();
 
   auto produce_path =
     pipe("produce_path") =
       recv(bus_socket)
       >> decode(as<message>)
       >> lock(bus_lock)
       >> route(channel_table)
       >> enqueue(message_buffer)
       >> notify(message_ready)
       >> unlock(bus_lock);
 
   auto consume_path =
     pipe("consume_path") =
       recv(bus_socket)
       >> decode(as<subscription>)
       >> lock(bus_lock)
       >> update(subscription_idx)
       >> unlock(bus_lock);
 
   auto route_path =
     pipe("route_path") =
       wait(message_ready)
       >> dequeue(message_buffer)
       >> lock(bus_lock)
       >> filter(subscription_idx)
       >> match_channels(channel_table)
       >> unlock(bus_lock)
       >> fanout(send(bus_socket));
 
   return std::make_tuple(bus_socket, channel_table, subscription_idx, bus_lock,
                          message_buffer, message_ready,
                          produce_path, consume_path, route_path);
 }
 
 } // namespace ipctk::stdipc
