cmake_minimum_required(VERSION 3.10)

project(Compiler)

# 设置编译器为clang++
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)

# 设置 Debug 模式（默认情况下，CMake 会生成 Debug 符号）
set(CMAKE_BUILD_TYPE Debug)  # 可选，也可以在命令行指定 `-DCMAKE_BUILD_TYPE=Debug`

# 添加源文件
file(GLOB FRONT_END_SOURCES 
    front-end/lib/*.cpp)
file(GLOB LLVM_SOURCES 
    llvm/lib/*.cpp 
    llvm/lib/value/*.cpp 
    llvm/lib/value/inst/*.cpp)
file(GLOB MIPS_SOURCES
    mips/lib/*.cpp)
add_executable(Compiler main.cpp 
                ${LLVM_SOURCES}
                ${FRONT_END_SOURCES}
                ${MIPS_SOURCES})

# 添加包含目录（此处为当前目录下的include文件夹下：${CMAKE_CURRENT_SOURCE_DIR}/include）
target_include_directories(Compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/front-end/include
    ${CMAKE_CURRENT_SOURCE_DIR}/llvm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/llvm/include/value
    ${CMAKE_CURRENT_SOURCE_DIR}/llvm/include/value/inst
    ${CMAKE_CURRENT_SOURCE_DIR}/mips/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置输出目录（此处为当前目录：${CMAKE_CURRENT_SOURCE_DIR}）
set_target_properties(Compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)