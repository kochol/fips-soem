cmake_minimum_required(VERSION 3.9)

fips_begin_lib(soem)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_TESTS TRUE)
else()
  message(STATUS "SOEM: not building tests when built as dependency")
  set(BUILD_TESTS FALSE)
endif()

if(WIN32)
  set(OS "win32")
  target_include_directories(soem PUBLIC oshw/win32/wpcap/Include)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(soem PUBLIC ${CMAKE_CURRENT_LIST_DIR}/oshw/win32/wpcap/Lib/x64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_directories(soem PUBLIC ${CMAKE_CURRENT_LIST_DIR}/oshw/win32/wpcap/Lib)
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /WX")
  set(OS_LIBS wpcap.lib Packet.lib Ws2_32.lib Winmm.lib)
elseif(UNIX AND NOT APPLE)
  set(OS "linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread rt)
elseif(APPLE)
  # This must come *before* linux or MacOSX will identify as Unix.
  set(OS "macosx")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread pcap)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rt-kernel")
  set(OS "rtk")
  message(STATUS "ARCH is ${ARCH}")
  message(STATUS "BSP is ${BSP}")
  include_directories(oshw/${OS}/${ARCH})
  file(GLOB OSHW_EXTRA_SOURCES oshw/${OS}/${ARCH}/*.c)
  set(OSHW_SOURCES "${OS_HW_SOURCES} ${OSHW_ARCHSOURCES}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format")
  set(OS_LIBS "-Wl,--start-group -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lc -lm -Wl,--end-group")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rtems")
  message(STATUS "Building for RTEMS")
  set(OS "rtems")
  set(SOEM_LIB_INSTALL_DIR ${LIB_DIR})
  set(BUILD_TESTS FALSE)
endif()

message(STATUS "OS is ${OS}")

file(GLOB SOEM_SOURCES soem/*.c)
file(GLOB OSAL_SOURCES osal/${OS}/*.c)
file(GLOB OSHW_SOURCES oshw/${OS}/*.c)

file(GLOB SOEM_HEADERS soem/*.h)
file(GLOB OSAL_HEADERS osal/osal.h osal/${OS}/*.h)
file(GLOB OSHW_HEADERS oshw/${OS}/*.h)

fips_files(
  ${SOEM_SOURCES}
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${OSHW_EXTRA_SOURCES})
  fips_libs(${OS_LIBS})
  message(${OS_LIBS})
fips_end_lib(soem)


target_include_directories(soem PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/soem>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal/${OS}>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/oshw/${OS}>
  $<INSTALL_INTERFACE:include/soem>
  )

if(BUILD_TESTS) 
  add_subdirectory(test/simple_ng)
  add_subdirectory(test/linux/slaveinfo)
  add_subdirectory(test/linux/eepromtool)
  add_subdirectory(test/linux/simple_test)
endif()
