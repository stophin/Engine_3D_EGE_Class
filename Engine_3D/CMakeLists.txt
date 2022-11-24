cmake_minimum_required(VERSION 3.8)
project(Engine3D)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-D_CRT_RAND_S)

set(EGE_PATH_EX F:/xege-master/)
set(EGE_PATH C:/Users/stophin/Source/ege19.01_all)

FILE(GLOB SRC_FILES
	./Engine_3D.cpp
	./common/*.cpp
	./flatform/*.cpp
	./math3d/*.cpp
	./raytracing/*.cpp
	./reader/*.cpp
	./scene/*.cpp
)

FILE(GLOB INC_FILES
	./common/*.h
	./flatform/*.h
	./math3d/*.h
	./raytracing/*.h
	./reader/*.h
	./scene/*.h
)

add_executable(Engine3D ${SRC_FILES} ${INC_FILES})

if (MINGW)
	target_include_directories(Engine3D
		PUBLIC ${EGE_PATH_EX}/src/
		#PUBLIC ${EGE_PATH}/include/
	)
	target_link_directories(Engine3D
		PUBLIC ./
		PUBLIC ${EGE_PATH_EX}/build/lib/MinGW
		#PUBLIC ${EGE_PATH}/lib/vs2019/lib/x64/
	)

	target_link_libraries(Engine3D  
		-lgraphics -lgdiplus -limm32 -lmsimg32 -lole32 -loleaut32 -lwinmm -luuid -mwindows -mconsole)
elseif (WIN32)
	target_include_directories(Engine3D
		PUBLIC ${EGE_PATH_EX}/src/
		#PUBLIC ${EGE_PATH}/include/
	)
	target_link_directories(Engine3D
		PUBLIC ./
		PUBLIC ${EGE_PATH_EX}/win/x64/$(Configuration)
		#PUBLIC ${EGE_PATH}/lib/vs2019/lib/x64/
	)
	target_link_libraries(Engine3D 
		-lgraphics64)
elseif (UNIX)
endif()


if(WIN32 AND NOT MINGW)
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)

	source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${SRC_FILES})
	source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Header Files" FILES ${INC_FILES})

	set_target_properties(Engine3D PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/../)

elseif(MINGW)
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/mingw)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/mingw)

	add_custom_target(run)
	add_custom_command(TARGET run PRE_BUILD COMMAND
		cd ../output/ && mingw\\Engine3D )
elseif(UNIX AND NOT APPLE)
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/linux/)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/linux/)

	add_custom_target(run)
	add_custom_command(TARGET run PRE_BUILD COMMAND
		cd ../output/ && ./linux/Engine3D )
endif()
