 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_channel(const std::string& socket_path) {
   using namespace ipctk::dsl;
 
   auto channel_socket = socket("channel_socket") = unix.listen(socket_path);
 
   auto backpressure  = signal("backpressure")  = eventfd();
   auto buffer_window = shared("buffer_window") = shm.open("/channel.buf", 64_KiB);
   auto channel_lock  = mutex("channel_lock")   = semaphore("/channel.lock", 1);
 
   auto send_path =
     pipe("send_path") =
       recv(channel_socket)
       >> encode(as<payload>)
       >> lock(channel_lock)
       >> frame(buffer_window)
       >> notify(backpressure)
       >> unlock(channel_lock)
       >> send(channel_socket);
 
   auto recv_path =
     pipe("recv_path") =
       recv(channel_socket)
       >> lock(channel_lock)
       >> deframe(buffer_window)
       >> unlock(channel_lock)
       >> decode(as<payload>)
       >> send(channel_socket);
 
   auto flow_control_path =
     pipe("flow_control_path") =
       wait(backpressure)
       >> lock(channel_lock)
       >> drain(buffer_window)
       >> unlock(channel_lock)
       >> encode(as<ack>)
       >> send(channel_socket);
 
   return std::make_tuple(channel_socket, backpressure, buffer_window, channel_lock,
                          send_path, recv_path, flow_control_path);
 }
 
 } // namespace ipctk::stdipc
