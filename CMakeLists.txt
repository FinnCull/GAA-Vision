cmake_minimum_required(VERSION 3.16)

project(gaa_vision VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)

add_executable(gaa_vision
    src/main.cpp
    src/tracker.cpp
)
target_link_libraries(gaa_vision
	        PRIVATE
		    ${OpenCV_LIBS}
	    )

		target_include_directories(gaa_vision
		PRIVATE
		include
		${OpenCV_INCLUDE_DIRS}
	)
target_compile_options(gaa_vision
		  PRIVATE
	 	 -Wall
		 -Wextra
		 -Wpedantic
    )

include(CTest)

if(BUILD_TESTING)
    add_executable(tracker_tests
        tests/tracker_test.cpp
        src/tracker.cpp
    )

    target_include_directories(tracker_tests
        PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(tracker_tests
        PRIVATE
        ${OpenCV_LIBS}
    )

    target_compile_options(tracker_tests
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(NAME tracker_tests COMMAND tracker_tests)
endif()
