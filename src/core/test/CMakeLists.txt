function (qs_test name)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} PRIVATE ${QT_DEPS} Qt6::Test quickshell-core)
	add_test(NAME ${name} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" COMMAND $<TARGET_FILE:${name}>)
endfunction()

qs_test(popupwindow popupwindow.cpp)
qs_test(transformwatcher transformwatcher.cpp)
qs_test(ringbuffer ringbuf.cpp)
