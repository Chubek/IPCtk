add_executable(example_zero_mq_like zero-mq-like.cpp)
target_link_libraries(example_zero_mq_like PRIVATE ipctk)

add_executable(example_simple_ipc_protocol simple-ipc-protocol.cpp)
target_link_libraries(example_simple_ipc_protocol PRIVATE ipctk)
