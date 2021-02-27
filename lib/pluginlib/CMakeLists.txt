cmake_minimum_required(VERSION 3.9)
project("Pluginlib")

file(COPY src/plugin.h DESTINATION include/hirzel)

# shared build
add_library(plugin SHARED "src/plugin.cpp")
target_link_libraries(plugin PRIVATE ${CMAKE_DL_LIBS})

# static build
add_library(plugin_static STATIC "src/plugin.cpp")
target_link_libraries(plugin_static PRIVATE ${CMAKE_DL_LIBS})

# static position independent build
add_library(plugin_staticpic STATIC "src/plugin.cpp")
target_link_libraries(plugin_staticpic PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(plugin_staticpic PROPERTIES POSITION_INDEPENDENT_CODE true)
