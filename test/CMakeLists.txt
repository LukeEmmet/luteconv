# pitch_test
add_executable(pitch_test pitch_test.cpp)
target_link_libraries(pitch_test
    luteconvlib
    ${ZLIB_LIBRARIES}
    ${ZIP_LIBRARY}
    ${PUGIXML_LIBRARY}
	${GTEST_LIBRARY}
)

add_test(pitch_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pitch_test)

# options_test
add_executable(options_test options_test.cpp)
target_link_libraries(options_test
    luteconvlib
    ${ZLIB_LIBRARIES}
    ${ZIP_LIBRARY}
    ${PUGIXML_LIBRARY}
	${GTEST_LIBRARY}
)

add_test(options_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/options_test)

# convert_test
add_definitions(-DBINARY_DIR=${CMAKE_BINARY_DIR} -DSOURCE_DIR=${CMAKE_SOURCE_DIR})
add_executable(convert_test convert_test.cpp)
target_link_libraries(convert_test
    luteconvlib
    ${ZLIB_LIBRARIES}
    ${ZIP_LIBRARY}
    ${PUGIXML_LIBRARY}
    ${GTEST_LIBRARY}
)

add_test(convert_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/convert_test)

