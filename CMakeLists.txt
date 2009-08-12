project(extragear-multimedia)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include(MacroOptionalAddSubdirectory)
find_package(RUBY)
find_package(KdeMultimedia)
find_package(OpenGL)
find_package(Xine)
# are these two really required ?
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)
set(TAGLIB_MIN_VERSION "1.5")
find_package(Taglib)

#amarok needs to be before add_definitions, since it builds
#some qt-only software
if(TAGLIB_FOUND AND RUBY_EXECUTABLE AND RUBY_INCLUDE_PATH)
   macro_optional_add_subdirectory(amarok)
endif(TAGLIB_FOUND AND RUBY_EXECUTABLE AND RUBY_INCLUDE_PATH)


add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include (ConfigureChecks.cmake)

if(TAGLIB_FOUND)
include_directories (${TAGLIB_INCLUDES})
endif(TAGLIB_FOUND)

if(KDEMULTIMEDIA_FOUND)
   macro_optional_add_subdirectory(k3b)
   macro_optional_add_subdirectory(kaudiocreator)
endif(KDEMULTIMEDIA_FOUND)
macro_optional_add_subdirectory(kaffeine)
macro_optional_add_subdirectory(kmid)
macro_optional_add_subdirectory(kplayer)
macro_optional_add_subdirectory(kmplayer)
macro_optional_add_subdirectory(doc)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND AND XINE_FOUND)
   macro_optional_add_subdirectory(kaffeinegl)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND AND XINE_FOUND)

