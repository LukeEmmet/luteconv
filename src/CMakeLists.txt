aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE)
list(REMOVE_ITEM SOURCE main.cpp)
add_library(luteconvlib STATIC ${SOURCE})

add_executable(luteconv ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(luteconv
	luteconvlib
	${ZLIB_LIBRARIES}
	${ZIP_LIBRARY}
	${PUGIXML_LIBRARY}
)
	
install(TARGETS luteconv
	RUNTIME
	DESTINATION "/usr/local/bin"
)
