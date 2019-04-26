cmake_minimum_required(VERSION 3.4)
project(gmenu2x LANGUAGES CXX VERSION 1.0)

include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

set(INSTALL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/gmenu2x"
	CACHE PATH "Installation directory for platform data")

option(CPUFREQ "Enable CPU frequency control" OFF)
if (CPUFREQ)
	add_compile_definitions(ENABLE_CPUFREQ)
endif(CPUFREQ)

option(CLOCK "Display current time at the bottom of the screen" ON)
if (CLOCK)
	add_compile_definitions(ENABLE_CLOCK)
endif (CLOCK)

option(BIND_CONSOLE "Support for binding/unbinding terminal" OFF)
if (BIND_CONSOLE)
	add_compile_definitions(BIND_CONSOLE)
endif (BIND_CONSOLE)

set(PLATFORMS "pc, a320, gcw0, nanonote, rs90")

set(PLATFORM "pc" CACHE STRING
	"One of the available platforms: ${PLATFORMS}"
)
set_property(CACHE PLATFORM PROPERTY STRINGS pc a320 gcw0 nanonote rs90)

if (PLATFORM MATCHES "a320")
	add_compile_definitions(PLATFORM_A320)
	set(SCREEN_RES "320x240")
elseif (PLATFORM MATCHES "gcw0")
	add_compile_definitions(PLATFORM_GCW0)
	set(SCREEN_RES "320x240")
elseif (PLATFORM MATCHES "rs90")
	add_compile_definitions(PLATFORM_RS90)
	set(SCREEN_RES "240x160")
elseif (PLATFORM MATCHES "nanonote")
	add_compile_definitions(PLATFORM_NANONOTE)
	set(SCREEN_RES "320x240")
elseif (PLATFORM MATCHES "pandora")
	add_compile_definitions(PLATFORM_PANDORA)
	set(SCREEN_RES "800x480")
elseif (PLATFORM MATCHES "pc")
	add_compile_definitions(PLATFORM_PC)
	set(SCREEN_RES "800x480")
else()
	message(FATAL_ERROR "PLATFORM must be one of the following: ${PLATFORMS}")
endif(PLATFORM MATCHES "a320")

add_compile_definitions(PLATFORM="${PLATFORM}")

find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(PNG REQUIRED)

find_library(LIBSDL_GFX_LIBRARY SDL_gfx)
find_path(LIBSDL_GFX_INCLUDE_DIR SDL_gfxPrimitives.h ${SDL_INCLUDE_DIR})
find_package_handle_standard_args(libSDL_gfx DEFAULT_MSG
	LIBSDL_GFX_LIBRARY LIBSDL_GFX_INCLUDE_DIR)

if(LIBSDL_GFX_FOUND)
	set(LIBSDL_GFX_LIBRARIES ${LIBSDL_GFX_LIBRARY})
	set(LIBSDL_GFX_INCLUDE_DIRS ${LIBSDL_GFX_INCLUDE_DIR})
else()
	message(FATAL_ERROR "Unable to find SDL_gfx")
endif(LIBSDL_GFX_FOUND)

find_library(LIBOPK_LIBRARY opk)
find_path(LIBOPK_INCLUDE_DIR opk.h /usr/include)
find_package_handle_standard_args(libopk DEFAULT_MSG
	LIBOPK_LIBRARY LIBOPK_INCLUDE_DIR)

if(LIBOPK_FOUND)
	set(LIBOPK_LIBRARIES ${LIBOPK_LIBRARY})
	set(LIBOPK_INCLUDE_DIRS ${LIBOPK_INCLUDE_DIR})
	add_compile_definitions(HAVE_LIBOPK)

	option(INOTIFY "Monitor OPK folder with inotify" ON)
	if (INOTIFY)
		add_compile_definitions(ENABLE_INOTIFY)
	endif (INOTIFY)
endif(LIBOPK_FOUND)

find_library(LIBXDGMIME_LIBRARY xdgmime)
find_path(LIBXDGMIME_INCLUDE_DIR xdgmime.h /usr/include)
find_package_handle_standard_args(libxdgmime DEFAULT_MSG
	LIBXDGMIME_LIBRARY LIBXDGMIME_INCLUDE_DIR)

if (LIBXDGMIME_FOUND)
	set(LIBXDGMIME_LIBRARIES ${LIBXDGMIME_LIBRARY})
	set(LIBXDGMIME_INCLUDE_DIRS ${LIBXDGMIME_INCLUDE_DIR})
	add_compile_definitions(HAVE_LIBXDGMIME)
endif(LIBXDGMIME_FOUND)

file(GLOB OBJS src/*.cpp)

add_executable(${PROJECT_NAME} ${OBJS})

target_link_libraries(${PROJECT_NAME} PRIVATE
					  ${SDL_LIBRARY}
					  ${SDL_TTF_LIBRARIES}
					  ${LIBSDL_GFX_LIBRARIES}
					  ${PNG_LIBRARIES}
					  ${LIBOPK_LIBRARIES}
					  ${LIBXDGMIME_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
						   ${SDL_INCLUDE_DIR}
						   ${SDL_TTF_INCLUDE_DIRS}
						   ${PNG_INCLUDE_DIRS}
						   ${LIBOPK_INCLUDE_DIRS}
						   ${LIBXDGMIME_INCLUDE_DIRS}
						   ${LIBSDL_GFX_INCLUDE_DIRS}
)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY data/platform/${PLATFORM}/ data/translations
		DESTINATION ${CMAKE_INSTALL_DATADIR}/gmenu2x
)
install(FILES data/about.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/gmenu2x)
install(DIRECTORY data/skins/${SCREEN_RES}/
		DESTINATION ${CMAKE_INSTALL_DATADIR}/gmenu2x/skins
)
