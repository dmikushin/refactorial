cmake_minimum_required(VERSION 3.16)

project(refactorial)

find_package(LLVM CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)
find_package(Clang CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/util.cpp
    src/AugmentedJSONCompilationDatabase.cpp
    # src/Transforms/AccessorsTransform.cpp
    # src/Transforms/ExtractParameterTransform.cpp
    # src/Transforms/IdentityTransform.cpp
    # src/Transforms/MethodMoveTransform.cpp
    src/Transforms/Qt3To5UIClasses.cpp
    src/Transforms/ExplicitConstructorTransform.cpp
    src/Transforms/NamedDeclMatcher.cpp
    src/Transforms/RenameTransform.cpp
    src/Transforms/ArgumentChange.cpp
    src/Transforms/Transforms.cpp
    src/Transforms/Replacer.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/Transforms)
target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE "-D_HAS_EXCEPTIONS=0")
if (NOT WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:${MSVC}>:/wd4624> $<$<BOOL:${MSVC}>:/wd4291> $<$<BOOL:${MSVC}>:/MT>)
target_link_directories(${PROJECT_NAME} PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE clangTooling clangToolingCore clangDriver clangFrontend
    clangFrontendTool clangParse clangSema clangEdit clangLex clangAnalysis clangRewrite clangRewriteFrontend
    clangSerialization clangAST clangASTMatchers clangDynamicASTMatchers clangBasic clangIndex)

