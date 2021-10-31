cmake_minimum_required(VERSION 3.17)
project(test C)

set(CMAKE_C_STANDARD 99)

link_libraries(setupapi.lib user32.lib ws2_32.lib)

add_executable(main main.c)
add_executable(host host.c)
add_executable(cTx cTx.c)
add_executable(cRx cRx.c)

add_executable(tx tx.c)
add_executable(rx rx.c)

add_executable(cAck cAck.c)
add_executable(test1 test1.c)
add_executable(c2 c2.c)
add_executable(c3 c3.c)
add_executable(thread_demo thread_demo.c)