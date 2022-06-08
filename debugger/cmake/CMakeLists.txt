cmake_minimum_required(VERSION 3.4.0)
project(riscvdebugger)

add_definitions(-DREPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}/..")

if(UNIX)
	set(EXECUTABLE_OUTPUT_PATH "linuxbuild/bin")
	set(INSTALL_DIR "linuxbuild/bin")
	add_compile_options(-rdynamic)
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

add_subdirectory(libdbg64g)
add_subdirectory(simple_plugin)
add_subdirectory(socsim_plugin)
# Build systemc plugin only on windows. Cannot link it properly on linux.
if (DEFINED ENV{SYSTEMC_SRC} AND DEFINED ENV{SYSTEMC_LIB})
    add_subdirectory(cpu_sysc_plugin)
else()
    message(WARNING "SYSTEMC_SRC and SYSTEMC_LIB are not set. SystemC cpu_sysc_plugin disabled")
endif()
add_subdirectory(cpu_arm_plugin)
add_subdirectory(cpu_fnc_plugin)
add_subdirectory(gui_plugin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/common
)


file(GLOB _riscvdebugger_src
	${CMAKE_CURRENT_SOURCE_DIR}/../src/common/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../src/appdbg64g/main.cpp
	)


add_executable(
   riscvdebugger
   ${_riscvdebugger_src}
)


if(UNIX)
    target_link_libraries(riscvdebugger pthread rt dl libdbg64g)

    add_custom_command(TARGET riscvdebugger
	        POST_BUILD
	        COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/post_build_event.sh "${CMAKE_CURRENT_BINARY_DIR}/linuxbuild/bin" "${CMAKE_CURRENT_SOURCE_DIR}"
	        )
else()

    foreach(_source IN ITEMS ${_riscvdebugger_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "..\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()


    set_target_properties(riscvdebugger PROPERTIES RUNTIME_OUTPUT_DIRECTORY "winbuild/bin")
    set_target_properties(riscvdebugger PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "winbuild/bin")
    set_target_properties(riscvdebugger PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "winbuild/bin")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT riscvdebugger)
    set_target_properties(riscvdebugger PROPERTIES
                                        VS_DEBUGGER_COMMAND_ARGUMENTS 
                                        "-c ${CMAKE_CURRENT_SOURCE_DIR}/../targets/func_river_x1_gui.json")
    add_custom_command(TARGET riscvdebugger
	        POST_BUILD
	        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build_event.bat "${CMAKE_CURRENT_BINARY_DIR}/winbuild/bin" "${CMAKE_CURRENT_SOURCE_DIR}"
	        )

    target_link_libraries(riscvdebugger libdbg64g)
if (DEFINED ENV{SYSTEMC_SRC} AND DEFINED ENV{SYSTEMC_LIB})
    add_dependencies(riscvdebugger libdbg64g simple_plugin socsim_plugin cpu_sysc_plugin cpu_arm_plugin cpu_fnc_plugin gui_plugin)
else()
    add_dependencies(riscvdebugger libdbg64g simple_plugin socsim_plugin cpu_arm_plugin cpu_fnc_plugin gui_plugin)
endif()
endif()
  
