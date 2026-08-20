 # stdipc — Standard Library of IPC Protocols for IPCtk
 # Header-only + data files; no compilation required.
 
 add_library(stdipc INTERFACE)
 target_include_directories(stdipc INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
 )
 target_link_libraries(stdipc INTERFACE ipctk)
 
 # Install native headers
 install(DIRECTORY native/
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stdipc/native
   FILES_MATCHING PATTERN "*.hpp"
 )
 
 # Install protocol .ipcl files
 install(DIRECTORY protocols/
   DESTINATION ${CMAKE_INSTALL_DATADIR}/stdipc/protocols
   FILES_MATCHING PATTERN "*.ipcl"
 )
 
 # Install primitive .ipcl files
 install(DIRECTORY primitives/
   DESTINATION ${CMAKE_INSTALL_DATADIR}/stdipc/primitives
   FILES_MATCHING PATTERN "*.ipcl"
 )
 
 # Install documentation
 install(FILES README.md
   DESTINATION ${CMAKE_INSTALL_DATADIR}/stdipc
 )
 
 # Convenience aggregate target: builds nothing but tracks all files
 add_custom_target(stdipc_all
   COMMENT "stdipc: Standard Library of IPC Protocols (header-only, no build)"
 )
