cmake_minimum_required(VERSION 3.13)

project(refactorial LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(LLVM CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)
find_package(Clang CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)
find_package(PCRECPP REQUIRED)

# Make sure we pick up the system-provided TBB, which is compatible with GCC,
# and not the idiotic oneAPI, which broke everything.
find_package(TBB PATHS "/usr/lib/x86_64-linux-gnu/cmake/TBB" REQUIRED NO_DEFAULT_PATH)

add_subdirectory(ThirdParty/xunused EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/util.cpp
    src/AugmentedJSONCompilationDatabase.cpp
    src/Transforms/AccessorsTransform.cpp
    # src/Transforms/ExtractParameterTransform.cpp
    # src/Transforms/IdentityTransform.cpp
    # src/Transforms/MethodMoveTransform.cpp
    src/Transforms/Qt3To5UIClasses.cpp
    src/Transforms/ExplicitConstructorTransform.cpp
    src/Transforms/NamedDeclRenamer.cpp
    src/Transforms/NamedDeclRemover.cpp
    src/Transforms/RenameTransform.cpp
    src/Transforms/RemoveTransform.cpp
    src/Transforms/ArgumentChange.cpp
    src/Transforms/Transforms.cpp
    src/Transforms/Replacer.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/Transforms)
target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} ${PCRECPP_INCLUDE_DIRS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0 TBB_USE_EXCEPTIONS=0)
if (NOT WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:${MSVC}>:/wd4624> $<$<BOOL:${MSVC}>:/wd4291> $<$<BOOL:${MSVC}>:/MT>)
target_link_directories(${PROJECT_NAME} PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE clangTooling clangToolingCore clangDriver clangFrontend
    clangFrontendTool clangParse clangSema clangEdit clangLex clangAnalysis clangRewrite clangRewriteFrontend
    clangSerialization clangAST clangASTMatchers clangDynamicASTMatchers clangBasic clangIndex LLVMSymbolize
    xunused_embedded ${PCRECPP_LIBRARIES} TBB::tbb)

add_custom_target(test_accessors
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/tests/Accessors -B${CMAKE_CURRENT_BINARY_DIR}/tests/Accessors -DREFACTORIAL=$<TARGET_FILE:${PROJECT_NAME}> -DCMAKE_CXX_COMPILER=${LLVM_TOOLS_BINARY_DIR}/clang++${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/Accessors --target test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/Accessors
    COMMENT "Testing Accessors")

add_custom_target(test_function_rename
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/tests/FunctionRename -B${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRename -DREFACTORIAL=$<TARGET_FILE:${PROJECT_NAME}> -DCMAKE_CXX_COMPILER=${LLVM_TOOLS_BINARY_DIR}/clang++${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRename --target test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRename
    COMMENT "Testing FunctionRename")

add_custom_target(test_function_remove
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/tests/FunctionRemove -B${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRemove -DREFACTORIAL=$<TARGET_FILE:${PROJECT_NAME}> -DCMAKE_CXX_COMPILER=${LLVM_TOOLS_BINARY_DIR}/clang++${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRemove --target test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tests/FunctionRemove
    COMMENT "Testing FunctionRemove")

add_custom_target(docker
    COMMAND docker build -f ${CMAKE_CURRENT_SOURCE_DIR}/docker/Dockerfile -t ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/docker
    COMMAND docker run -it --rm --user "${UID}:${GID}" -v${CMAKE_CURRENT_SOURCE_DIR}:/project ${PROJECT_NAME} sh /project/docker/scripts/build.sh
    COMMENT "Building ${PROJECT_NAME} in a Docker container")

