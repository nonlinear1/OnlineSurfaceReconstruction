cmake_minimum_required (VERSION 3.1)
project(OnlineSurfaceReconstruction)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

FUNCTION(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
ENDFUNCTION()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(ADDITIONAL_SRC)
set(LIBS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/")
include(SetupBuildEnvironment)

# Add NanoGUI dependency
set(NANOGUI_USE_GLAD ON)
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  ON  CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_EIGEN_INCLUDE_DIR ${EIGEN3_INCLUDE_DIR})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/nanogui ${CMAKE_BINARY_DIR}/ext_build/nanogui)
set_property(TARGET nanogui nanogui-obj glfw glfw_objects PROPERTY FOLDER "dependencies")
include_directories(ext/nanogui/include
				${NANOGUI_EXTRA_INCS})
set(LIBS ${LIBS} nanogui ${NANOGUI_EXTRA_LIBS})
add_definitions(${NANOGUI_EXTRA_DEFS})

# Configure additional features of NSEssentials (will be included by libOSR)
SET(NSE_SSBO_SUPPORT ON CACHE BOOL " " FORCE)
SET(NSE_TESSELLATION_SUPPORT ON CACHE BOOL " " FORCE)
SET(NSE_WITH_NANOGUI ON CACHE BOOL " " FORCE)

# Add libOSR
add_subdirectory(libOSR ${CMAKE_BINARY_DIR}/ext_build/libOSR)
set(LIBS OSR nsessentials ${LIBS})
include_directories(libOSR/include ${OSR_ADDITIONAL_INCLUDE_DIR})
add_definitions(${OSR_EXTRA_DEFS})

set(GLSL_FILES 	mesh.vert
				mesh_flat.vert
				blinnphong.frag
				flat_shading.geom
				void.vert
				mesh_colors_tri.tcs mesh_colors_tri.tes
				mesh_colors_quad.tcs mesh_colors_quad.tes
				edges.vert edges.tcs edges.tes
				orientation_field.frag orientation_field.geom orientation_field.vert
				normal.frag normal.geom normal.vert
				position_field.frag position_field.vert
				lines.vert lines.frag
				sphere.vert sphere.geom sphere.frag
				phantom.vert phantom.frag)

PREPEND(GLSL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/glsl/" ${GLSL_FILES})

source_group(glsl FILES ${GLSL_FILES})

JOIN("${GLSL_FILES}" "," glsl_string)
set(bin2c_cmdline
    -DOUTPUT_C=glsl.cpp
    -DOUTPUT_H=glsl.h
    "-DINPUT_FILES=${glsl_string}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/ext/nanogui/resources/bin2c.cmake")

add_custom_command(
  OUTPUT glsl.cpp glsl.h
  COMMAND ${CMAKE_COMMAND} ARGS ${bin2c_cmdline}
  DEPENDS ${GLSL_FILES}
  COMMENT "Running bin2c"
  PRE_BUILD VERBATIM)

SET(GUI_FILES	src/osr/gui/Viewer.cpp include/osr/gui/Viewer.h				
				include/osr/gui/GUIObject.h								
				src/osr/gui/HierarchyRenderer.cpp include/osr/gui/HierarchyRenderer.h
				src/osr/gui/ExtractedMeshGL.cpp include/osr/gui/ExtractedMeshGL.h
				src/osr/gui/ShaderPool.cpp include/osr/gui/ShaderPool.h
				src/osr/gui/ScanRenderer.cpp include/osr/gui/ScanRenderer.h
				src/osr/gui/DataGL.cpp include/osr/gui/DataGL.h)				
source_group(gui FILES ${GUI_FILES})

SET(GUI_TOOL_FILES
				include/osr/gui/tools/Tool.h
				src/osr/gui/tools/FillHoleTool.cpp include/osr/gui/tools/FillHoleTool.h
				src/osr/gui/tools/SmoothTool.cpp include/osr/gui/tools/SmoothTool.h
				src/osr/gui/tools/RemoveTool.cpp include/osr/gui/tools/RemoveTool.h
				src/osr/gui/tools/ManualCoarseRegistrationTool.cpp include/osr/gui/tools/ManualCoarseRegistrationTool.h
				src/osr/gui/tools/Selection.cpp include/osr/gui/tools/Selection.h)
			
source_group(gui\\tools FILES ${GUI_TOOL_FILES})
			
SET(GUI_LOADERS_FILES
				include/osr/gui/loaders/ScanLoader.h
				src/osr/gui/loaders/FileScanLoader.cpp include/osr/gui/loaders/FileScanLoader.h
				src/osr/gui/loaders/ProceduralScanLoader.cpp include/osr/gui/loaders/ProceduralScanLoader.h)

if(OSR_USE_DAVIDVIVE)
	set(GUI_LOADERS_FILES ${GUI_LOADERS_FILES} 
		src/osr/gui/loaders/DavidViveScanLoader.cpp include/osr/gui/loaders/DavidViveScanLoader.h 
		src/osr/gui/loaders/DavidTurntableConnection.cpp include/osr/gui/loaders/DavidTurntableConnection.h)

	add_definitions (/DUSE_DAVIDVIVE)
	find_package(OpenVR REQUIRED)
	include_directories(${OPENVR_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${OPENVR_LIB})

	set(DAVIDVIVE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DavidVive/TakeDavidScan.au3" "${CMAKE_CURRENT_SOURCE_DIR}/DavidVive/viveController.ply")

	JOIN("${DAVIDVIVE_FILES}" "," davidvive_string)
	set(bin2c_cmdline
		-DOUTPUT_C=davidviveres.cpp
		-DOUTPUT_H=davidviveres.h
		"-DINPUT_FILES=${davidvive_string}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/ext/nanogui/resources/bin2c.cmake")

	add_custom_command(
	  OUTPUT davidviveres.cpp davidviveres.h
	  COMMAND ${CMAKE_COMMAND} ARGS ${bin2c_cmdline}
	  DEPENDS ${DAVIDVIVE_FILES}
	  COMMENT "Running bin2c"
	  PRE_BUILD VERBATIM)

	set(ADDITIONAL_SRC ${ADDITIONAL_SRC} davidviveres.cpp)
endif(OSR_USE_DAVIDVIVE)

source_group(gui\\loaders FILES ${GUI_LOADERS_FILES})				

include_directories(include ${CMAKE_BINARY_DIR})

add_executable(OnlineSurfaceReconstruction MACOSX_BUNDLE
	glsl.cpp
	src/main.cpp	
	src/osr/BatchSession.cpp include/osr/BatchSession.h
	${GUI_FILES}
	${GUI_TOOL_FILES}
	${GUI_LOADERS_FILES}
	${ADDITIONAL_SRC}
	${GLSL_FILES})
	
#add_custom_command(TARGET OnlineSurfaceReconstruction POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_BINARY_DIR}/ext_build/libOSR/$<CONFIG>"
#        $<TARGET_FILE_DIR:OnlineSurfaceReconstruction>)

target_link_libraries(OnlineSurfaceReconstruction ${LIBS})