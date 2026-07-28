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
