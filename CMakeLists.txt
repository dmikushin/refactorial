CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (Refactorial)

# debian packet dependencies:
#
# sudo apt-get install libyaml-cpp-dev clang
#
# also need llvm+clang in version 3.2:
# {{{
#     export BRANCH=release_32
#     git clone http://llvm.org/git/llvm.git $HOME/llvm.git --branch $BRANCH
#     git clone http://llvm.org/git/clang.git $HOME/llvm.git/tools/clang --branch $BRANCH
#     mkdir -p $HOME/llvm.git/build && cd $HOME/llvm.git/build
#     cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=true -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/llvm.install -DLLVM_TARGETS_TO_BUILD="X86"
#     make -j`getconf _NPROCESSORS_ONLN` all install
# }}}
#
# compiling with gcc 4.8 seems to break, clang 3.3 works. do the following in your shell:
# {{{
#     export CC=clang
#     export CXX=clang++
#     mkdir build && cd build
#     cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
#     make
# }}}

add_definitions("-D_HAS_EXCEPTIONS=0")

if(NOT WIN32)
  # kinda hardcoded by trial-and-error...
  add_definitions("-fno-rtti -Wno-c++11-extensions -Wno-nested-anon-types")
  add_definitions("-g")
endif()

macro(llvm_get_config OUTPUT)
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} ${ARGN}
        RESULT_VARIABLE __llvm_ok
        OUTPUT_VARIABLE ${OUTPUT})
    if (NOT __llvm_ok STREQUAL 0)
        message(FATAL_ERROR " -- llvm_get_config: failed to execute ${LLVM_CONFIG_EXECUTABLE} ${ARGN} (result: ${__llvm_ok})")
    endif()
    string(STRIP ${OUTPUT} "${${OUTPUT}}")
    string(REPLACE "\n" " " ${OUTPUT} "${${OUTPUT}}")
endmacro()

macro(resolve_library_flags VARIABLE)
    foreach(${VARIABLE}_lib ${${VARIABLE}_LIBRARIES})
      set(_${VARIABLE}_lib NOTFOUND)
      string(REPLACE "^-l" "" ${VARIABLE}_lib ${${VARIABLE}_lib})
      find_library(_${VARIABLE}_lib NAMES ${${VARIABLE}_lib} HINTS ${${VARIABLE}_LIBRARY_DIRS})
      if (NOT _${VARIABLE}_lib)
        set(_${VARIABLE}_lib ${${VARIABLE}_lib})
      endif()
      list(APPEND _${VARIABLE}_LIBRARIES ${_${VARIABLE}_lib})
    endforeach()
    list(APPEND _${VARIABLE}_LIBRARIES ${${VARIABLE}_LDFLAGS_OTHER})
    set(${VARIABLE}_LIBRARIES ${_${VARIABLE}_LIBRARIES} CACHE INTERNAL "")
endmacro()

# kinda hardcoded for my setup...
find_program(LLVM_CONFIG_EXECUTABLE
    NAMES llvm-config
    PATHS "~/llvm.install/bin" "C:/llvm/bin"
    )
# check that
if(${LLVM_CONFIG_EXECUTABLE} MATCHES NOTFOUND)
    message(FATAL_ERROR " -- could not find llvm-config")
else()
    llvm_get_config(LLVM_VERSION "--version")
    message("-- got llvm-config: '${LLVM_CONFIG_EXECUTABLE}', version ${LLVM_VERSION}")
endif(${LLVM_CONFIG_EXECUTABLE} MATCHES NOTFOUND)

# crate this list: make it empty, wait for "undefined reference" errors, note a
# symbol and call:
#   nm -C -A ~/llvm.install/lib/* | grep -e "[rRtT]\s*llvm::MCContext::~MCContext"
# note: don't link "clang" which is a shared lib. you'll get doubled symbols
list(APPEND CLANG_LIBRARIES
  clangTooling
  clangToolingCore
  clangDriver
  clangFrontend
  clangFrontendTool
  clangParse
  clangSema
  clangEdit
  clangLex
  clangAnalysis
  clangRewrite
  clangRewriteFrontend
  clangSerialization
  clangAST
  clangASTMatchers
  clangDynamicASTMatchers
  clangBasic
  clangIndex
  )

llvm_get_config(LLVM_CXXFLAGS "--cxxflags")

llvm_get_config(LLVM_LIBRARY_DIRS "--libdir")
string(STRIP ${LLVM_LIBRARY_DIRS} LLVM_LIBRARY_DIRS)

llvm_get_config(LLVM_RAW_LIBRARIES "--libs" "--system-libs")
string(REPLACE "-l" "" LLVM_RAW_LIBRARIES "${LLVM_RAW_LIBRARIES}")
string(STRIP ${LLVM_RAW_LIBRARIES} LLVM_RAW_LIBRARIES)
string(REPLACE " " ";" LLVM_RAW_LIBRARIES "${LLVM_RAW_LIBRARIES}")

if(WIN32)
  # FIXME: llvm-config returns improper flags for Windows in 3.8
  foreach(l ${LLVM_RAW_LIBRARIES})
	LIST(APPEND LLVM_LIBRARIES ${l}.lib)
  endforeach()
else()
  set(LLVM_LIBRARIES ${LLVM_RAW_LIBRARIES})
endif()

if(NOT WIN32)
  # FIXME: This provides incorrect flags for Windows in 3.8
  llvm_get_config(LLVM_LDFLAGS "--ldflags")
endif()

set(ADDL_CXXFLAGS "")
if(NOT WIN32)
  # FIXME: LLVM adds the wrong flag for Clang. Remove it and add the right one.
  string(REPLACE "-Wno-maybe-uninitialized" "-Wno-uninitialized" LLVM_CXXFLAGS "${LLVM_CXXFLAGS}")
else()
  # FIXME: LLVM wants to add /Ob2 which conflicts with flags added based on build type. Let the build type determine
  # desired inlining behavior.
  string(REPLACE "/Ob2" "" LLVM_CXXFLAGS "${LLVM_CXXFLAGS}")
  set(ADDL_CXXFLAGS "${ADDL_CXXFLAGS} /WX")
endif()

set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} ${ADDL_CXXFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LDFLAGS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  llvm_get_config(LLVM_INCLUDEDIR "--includedir")
  include_directories(${LLVM_INCLUDEDIR})

  set(ADDITIONAL_LIBRARIES "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/Version.lib")
else()
  set(ADDITIONAL_LIBRARIES "dl" "pthread")
endif()

link_directories(${LLVM_LIBRARY_DIRS})

add_executable(refactorial
    main.cpp
    util.cpp
    AugmentedJSONCompilationDatabase.cpp
    # Transforms/AccessorsTransform.cpp
    # Transforms/ExtractParameterTransform.cpp
    # Transforms/IdentityTransform.cpp
    # Transforms/MethodMoveTransform.cpp
	Transforms/Qt3To5UIClasses.cpp
	Transforms/ExplicitConstructorTransform.cpp
    Transforms/NamedDeclMatcher.cpp
    Transforms/RenameTransform.cpp
    Transforms/Transforms.cpp
    Transforms/Replacer.cpp
	)

target_link_libraries(refactorial
    ${CLANG_LIBRARIES}
    ${LLVM_LIBRARIES}
    ${ADDITIONAL_LIBRARIES}
    )

# debug crazy system-header-indlude-problems in linked clang:
#    echo '#include <stdarg.h> \nint main(){return 0;}' | ~/llvm.install/bin/clang -xc -v -
# but this targets "clang", where the libclang might be different
#
