 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_mailbox(const std::string& listen_addr, int port) {
   using namespace ipctk::dsl;
 
   auto mailbox_socket = socket("mailbox_socket") = tcp.listen(listen_addr + ":" + std::to_string(port));
 
   auto mailbox_storage = shared("mailbox_storage") = shm.open("/mailboxes", 512_KiB);
   auto mailbox_lock    = mutex("mailbox_lock")     = semaphore("/mailbox.lock", 1);
   auto arrival_signal  = signal("arrival_signal")  = eventfd();
 
   auto send_path =
     pipe("send_path") =
       recv(mailbox_socket)
       >> decode(as<message>)
       >> lock(mailbox_lock)
       >> deposit(mailbox_storage)
       >> notify(arrival_signal)
       >> unlock(mailbox_lock);
 
   auto receive_path =
     pipe("receive_path") =
       wait(arrival_signal)
       >> lock(mailbox_lock)
       >> retrieve(mailbox_storage)
       >> remove(mailbox_storage)
       >> unlock(mailbox_lock)
       >> decode(as<payload>)
       >> send(mailbox_socket);
 
   auto poll_path =
     pipe("poll_path") =
       recv(mailbox_socket)
       >> decode(as<poll_request>)
       >> lock(mailbox_lock)
       >> peek(mailbox_storage)
       >> unlock(mailbox_lock)
       >> encode(as<count>)
       >> send(mailbox_socket);
 
   return std::make_tuple(mailbox_socket, mailbox_storage, mailbox_lock, arrival_signal,
                          send_path, receive_path, poll_path);
 }
 
 } // namespace ipctk::stdipc
