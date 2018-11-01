cmake_minimum_required(VERSION 2.8)
 
PROJECT(VTKOpticalFlow)

find_package(VTK REQUIRED)

include(${VTK_USE_FILE})

file(GLOB HEADER_FILES *.h *.hxx)
file(GLOB SOURCE_FILES *.cpp *cxx)

if (WIN32)
	add_executable(VTKOpticalFlow WIN32 ${HEADER_FILES} ${SOURCE_FILES})
	if(MSVC)
		set_target_properties(VTKOpticalFlow PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
		set_target_properties(VTKOpticalFlow PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
	endif()
	#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
else(APPLE)
	add_executable(VTKOpticalFlow MACOSX_BUNDLE ${HEADER_FILES} ${SOURCE_FILES})
endif()

target_link_libraries(VTKOpticalFlow ${VTK_LIBRARIES})