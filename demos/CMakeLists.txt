cmake_minimum_required(VERSION 3.25)

project(dtsdemo)

# Define files to be compiled
set(MISC_FILES
    stream_demo.cpp
    data_dump.cpp
    bench.cpp
)

# Build and link all executables:

foreach( utilfile ${MISC_FILES} )

    # Determine a target name

    string( REPLACE ".cpp" "" utilname ${utilfile} )

    # Add the executible for this test:

    add_executable( ${utilname} ${utilfile} )

    # Link ptusdk

    target_link_libraries( ${utilname} dts )

endforeach( utilfile ${MISC_FILES} )
