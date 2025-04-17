

set(test_src
        linker_test.cpp
        trace_test.cpp
        test.cpp
)

add_library(qbdi-tracer-test SHARED ${test_src})
target_link_libraries(qbdi-tracer-test PRIVATE qbdi-tracer-static)