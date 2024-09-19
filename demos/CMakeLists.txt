cmake_minimum_required(VERSION 3.25)

project(dtsdemo)

# Define executable:

add_executable(${PROJECT_NAME}
    stream_demo.cpp
)

# Link in DTS

target_link_libraries(${PROJECT_NAME} PRIVATE
    dts
)
