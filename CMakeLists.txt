cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsEngineGuide CXX)

set(QT_PREFIX_PATH $ENV{QT6DIR64}) # QTDIR64
message("QT install path: $ENV{QT6DIR64}") # QTDIR64
list(APPEND CMAKE_PREFIX_PATH ${QT_PREFIX_PATH})

# 禁用cmakes时Qt私有模块警告，见QtPublicDependencyHelpers.cmake
set(QT_NO_PRIVATE_MODULE_WARNING ON)

set(__QT_DEPLOY_TOOL "D:/Program/qt6/bin/windeployqt.exe")

set(ISWINDOWS OFF)
set(ISLINUX OFF)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(ISWINDOWS ON)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(ISLINUX ON)
else()
	MESSAGE(FATAL_ERROR "Has not support your system:${CMAKE_SYSTEM_NAME}...")
endif()

if(${ISWINDOWS})
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(PROJECT_PLATFORM "Win32")
		set(PROJECT_PLATFORM_X "X86")
		add_definitions(-DWIN32)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(PROJECT_PLATFORM "x64")
		set(PROJECT_PLATFORM_X "X64")
		add_definitions(-DWIN64)
	endif()
else()
    set(PROJECT_PLATFORM "x64")
    set(PROJECT_PLATFORM_X "X64")
endif()

##源代码位置
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

##将动态库\静态库输出路径定位到特定路径,供调试时使用(否则依赖的库分布在各个文件夹)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_PLATFORM}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/lib/${PROJECT_PLATFORM}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/lib/${PROJECT_PLATFORM}")

set(INSTALLATION_PACKAGE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/InstallationPackage)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGEDITOR Debug Release)
find_package(Qt6 COMPONENTS Core Widgets Gui Multimedia ShaderTools GuiPrivate WidgetsPrivate ShaderToolsPrivate MultimediaPrivate REQUIRED)

function(add_shader TARGET_NAME SHADER_PATH)
    set(OUTPUT_SHADER_PATH ${SHADER_PATH}.qsb)  #输出文件路径
    add_custom_command(
        OUTPUT ${OUTPUT_SHADER_PATH}        #指定输出文件
        COMMAND qsb.exe -c ${SHADER_PATH} -o ${OUTPUT_SHADER_PATH} --glsl 430 --msl 12 --hlsl 60    #执行QSB工具
        MAIN_DEPENDENCY ${SHADER_PATH}      #指定依赖文件，即该文件变动时，触发上述命令
    )  
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY SOURCES ${OUTPUT_SHADER_PATH})               #需要把输出文件添加到一个构建目标中，才会触发CustomCommand
    source_group("Shader Files" FILES ${SHADER_PATH} ${OUTPUT_SHADER_PATH})                         #将着色器文件分类
endfunction()

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCES FILES  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.h ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} WIN32
        ${PROJECT_SOURCES}
    )
    target_link_libraries(${EXAMPLE_NAME} PRIVATE QEngineCore) # QEngineLaunch
    #add_dependencies(${EXAMPLE_NAME} QEngineCopyDLL)
    
    add_custom_command( #设置生成事件>>生成后事件 参考cmake\linux\helpers.cmake。拷贝Resources文件夹;调用windeployqt部署qt的dll
      TARGET ${EXAMPLE_NAME}
      POST_BUILD
      COMMAND
        "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
        "${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_PLATFORM}/$<CONFIG>/Resources"
      COMMAND
        "${__QT_DEPLOY_TOOL}" "${EXAMPLE_NAME}.exe" WORKING_DIRECTORY
        "${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_PLATFORM}/$<CONFIG>"
      COMMENT "Copy ${EXAMPLE_NAME} to binary directory"
      VERBATIM
    )
endfunction()

function(add_example_dir DIR_PATH GROUP_NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${DIR_PATH} ${DIR_PATH}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(NOT EXISTS "${DIR_PATH}/${EXAMPLE_NAME}/Source")
            add_example_dir(${DIR_PATH}/${EXAMPLE_NAME} ${GROUP_NAME}/${EXAMPLE_NAME})
        elseif(IS_DIRECTORY ${DIR_PATH}/${EXAMPLE_NAME})
            add_example(${DIR_PATH}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${GROUP_NAME})   
            install(TARGETS ${EXAMPLE_NAME} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})
        endif()
    endforeach()
endfunction()

add_subdirectory(Source/0-QEngineUtilities)
file(GLOB PLUGIN_DIR_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Source/Plugins/*)
#foreach(PLUGIN_DIR ${PLUGIN_DIR_LIST})
#    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_DIR})
#        add_subdirectory(${PLUGIN_DIR})
#    endif()
#endforeach()
#qengine_make_copy_target(QEngineCopyDLL)

add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/1-GraphicsAPI 1-GraphicsAPI)
#add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/2-EngineTechnology 2-EngineTechnology)
#add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/3-GraphicsTechnology 3-GraphicsTechnology)

# 部分示例的特殊操作
#add_shader(03-Shader ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/color.frag)
#set_property(TARGET 00-Blur PROPERTY AUTOMOC ON)
#set_property(TARGET 01-Bloom PROPERTY AUTOMOC ON)
#set_property(TARGET 02-Outlining PROPERTY AUTOMOC ON)
#set_property(TARGET 03-SSAO PROPERTY AUTOMOC ON)
#set_property(TARGET 04-DepthOfField PROPERTY AUTOMOC ON)
#set_property(TARGET 00-RenderingArchitecture PROPERTY AUTOMOC ON)
#set_property(TARGET 05-GPUParticles PROPERTY AUTOMOC ON)
#set_property(TARGET 03-SSAO PROPERTY AUTOMOC ON)



#execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${CMAKE_CURRENT_BINARY_DIR}/Resources)

get_property(QENGINE_TARGET_LIST GLOBAL PROPERTY QENGINE_TARGET_LIST)
foreach(QENGINE_TARGET ${QENGINE_TARGET_LIST})
    get_target_property(PLUGIN_TYPE ${QENGINE_TARGET} PLUGIN_TYPE)
    if(PLUGIN_TYPE)
        install(TARGETS ${QENGINE_TARGET} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR}/Plugins)
    else()
        install(TARGETS ${QENGINE_TARGET} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})
    endif()
endforeach()

install(DIRECTORY Resources DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})

install(CODE "execute_process(COMMAND windeployqt 00-Blur.exe WORKING_DIRECTORY ${INSTALLATION_PACKAGE_OUTPUT_DIR})")
