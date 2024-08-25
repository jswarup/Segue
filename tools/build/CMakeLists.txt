# CMakeLists.txt ____________________________________________________________________________________________________________

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)   
set( CMAKE_VERBOSE_MAKEFILE TRUE) 

set( TR_VERSIONID "1.0") 
set( TR_PROJECT Trellis)
project( ${TR_PROJECT} VERSION ${TR_VERSIONID} DESCRIPTION "Trellis Framework") 

#____________________________________________________________________________________________________________________________ 

enable_language( CXX) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 
add_compile_options("$<$<CONFIG:DEBUG>:-DTR_DEBUG>")

#____________________________________________________________________________________________________________________________ 
# Variables to be externally specified
 
option( TR_TRACEDBG "enable tracing" OFF)  
option( TR_SANITY "enable SANITY" OFF)   
 
if (TR_TRACEDBG) 
   add_definitions(-TR_TRACEDBG)
endif()

if (TR_SANITY) 
	add_definitions(-DTR_SANITY) 
endif() 

if (WIN32)
    add_definitions(-DTR_WINDOWS)
endif()
if (MSVC)
    add_definitions(/bigobj)
endif() 
 
#____________________________________________________________________________________________________________________________

message(STATUS "${CMAKE_PROJECT_NAME}: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_BUILD_TYPE}\n")

set(ROOT_PATH "${CMAKE_SOURCE_DIR}/../..")    
set(SOURCE_PATH "${ROOT_PATH}/src") 
set(TOOLS_PATH "${ROOT_PATH}/tools")  
set(THRDPARTY_PATH "${ROOT_PATH}/3rdparty")   
set(WORKDIR_PATH "${ROOT_PATH}/workdir")    
set(LIBDIR_PATH "${ROOT_PATH}/lib")  
set(BIN_PATH "${CMAKE_CURRENT_BINARY_DIR}")

#______________________________________________________________________________________________________________________________

execute_process(COMMAND
	"git" describe --match=NeVeRmAtChAnyStRiNg --always --abbrev=40 --dirty
	WORKING_DIRECTORY "${ROOT_PATH}" 
	OUTPUT_VARIABLE   GIT_SHA1
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)  
 
FILE(WRITE  ${CMAKE_BINARY_DIR}/tr_fingerprint.txt ${GIT_SHA1})

set( TR_VERSION ${TR_VERSIONID}-${GIT_SHA1} ) 
add_definitions(-DTR_VERSION="${TR_VERSION}")  

#____________________________________________________________________________________________________________________________

find_package(Python 3.10  COMPONENTS Interpreter Development)

message("Python_VERSION:${Python_VERSION}")
message("Python_INCLUDE_DIRS:${Python_INCLUDE_DIRS}")
message("Python_LIBRARIES:${Python_LIBRARIES}")
message("Python_library: ${PYTHON_LIBRARY}")
if (EXISTS ${THRDPARTY_PATH}/nanobind/CMakeLists.txt)
	if ( ${Python_FOUND} AND ${Python_Development_FOUND} )
		list(APPEND CMAKE_PREFIX_PATH ${THRDPARTY_PATH}/nanobind)
		find_package(nanobind CONFIG REQUIRED) 
		set(THRDPARTY_LIBS ${THRDPARTY_LIBS} asmjit::asmjit)
		set ( TR_PYDEV TRUE)
	endif()
else()
	message("Missing submodule: nanobind @ ${THRDPARTY_PATH}") 
endif()

#____________________________________________________________________________________________________________________________
  
if (EXISTS ${THRDPARTY_PATH}/asmjit/CMakeLists.txt)  
	set(ASMJIT_STATIC TRUE)		
	set(ASMJIT_TEST  FALSE)
	include("${THRDPARTY_PATH}/asmjit/CMakeLists.txt") 
else()
	message("Missing submodule: asmjit @ ${THRDPARTY_PATH}") 
endif()

#____________________________________________________________________________________________________________________________

set(CMAKE_INCLUDE_CURRENT_DIR ON)  
 
set(THRDPARTY_INCLUDES ${THRDPARTY_INCLUDES} ${THRDPARTY_PATH}/lz4)
 
include_directories( ${SOURCE_PATH}  /usr/local/include ${THRDPARTY_INCLUDES})
link_directories(/usr/local/lib ${CMAKE_BINARY_DIR}/lib )
install( FILES ${THRDPARTY_LIB}  DESTINATION ${INSTALL_PATH})

#____________________________________________________________________________________________________________________________

set( TR_SISYPHUS_SOURCES 
	${SOURCE_PATH}/sisyphus/tr_consoleapp.cpp	 
)

add_executable( trellis ${TR_SISYPHUS_SOURCES} )  
set_target_properties( trellis PROPERTIES DEBUG_POSTFIX "d") 
target_link_libraries( trellis  ${THRDPARTY_LIBS})
add_custom_command( TARGET trellis POST_BUILD
                   COMMAND  echo  ${CMAKE_PROJECT_NAME}: ${CMAKE_CXX_COMPILER_ID} $<CONFIG>  $<TARGET_FILE_NAME:trellis> done
)

install( TARGETS trellis DESTINATION ${INSTALL_PATH})

#____________________________________________________________________________________________________________________________
