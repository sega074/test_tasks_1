cmake_minimum_required(VERSION 3.0)

project(task1 LANGUAGES C)

add_executable(task1 main.c timer_tick.c resive_msg.c console_mgm.c send_req.c)

target_link_libraries(task1 rt)

install(TARGETS task1 RUNTIME DESTINATION bin)
