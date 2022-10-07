cmake_minimum_required(VERSION 3.8)
project(Engine3D)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EGE_PATH_EX C:/Users/stophin/Source/github/xege.git/branches/xege-471b90e638f760846a3d6192c3f05be98b12f348)
set(EGE_PATH C:/Users/stophin/Source/ege19.01_all)
set(DX_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
#add_definitions(-D_USING_GRAPHICS_QT)
add_definitions(-D_USING_GRAPHICS_EGE)
#add_definitions(-D_USING_GRAPHICS_DX)
#add_definitions(-D_USING_GRAPHICS_GDI)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)

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
source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Header Files" FILES ${INC_FILES})

add_executable(Engine3D ${SRC_FILES} ${INC_FILES})

target_include_directories(Engine3D
	PUBLIC ${EGE_PATH_EX}/include/
	#PUBLIC ${EGE_PATH}/include/
	PUBLIC ${DX_PATH}/Include/
)
target_link_directories(Engine3D
	PUBLIC ./
	PUBLIC ${EGE_PATH_EX}/lib/vs2019/x64/
	#PUBLIC ${EGE_PATH}/lib/vs2019/lib/x64/
	PUBLIC ${DX_PATH}/Lib/x64/
)
target_link_libraries(Engine3D optimized ucrt.lib)
target_link_libraries(Engine3D d3d11.lib)
target_link_libraries(Engine3D 
	debug d3dx11d.lib optimized d3dx11.lib)
target_link_libraries(Engine3D 
	debug d3dx10d.lib optimized d3dx10.lib)
target_link_libraries(Engine3D gdiplus.lib)

set_target_properties(Engine3D PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/../)