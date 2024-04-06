cmake_minimum_required(VERSION 3.10)
project(MyAudioProject)

# Указать пути к SFML и sndfile
set(SFML_DIR "S:\\C++ libraries\\SFML-2.6.0-windows-vc17-64-bit\\SFML-2.6.0\\lib\\cmake\\SFML")
list(APPEND CMAKE_PREFIX_PATH "S:/C++ libraries/libsndfile-1.2.2-win64/libsndfile-1.2.2-win64/cmake")

# Найти библиотеки SFML и sndfile
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(sndfile REQUIRED)
link_directories("S:/C++ libraries/libsndfile-1.2.2-win64/libsndfile-1.2.2-win64/lib")

# Указать директории с заголовочными файлами
include_directories(
    include
    "S:\\C++ libraries\\libsndfile-1.2.2-win64\\libsndfile-1.2.2-win64\\include"
)

# Указать исходные файлы для компиляции
set(SOURCE_FILES src/main.cpp src/tinyfiledialogs.cpp src/SliderSFML.cpp) # и другие .cpp файлы

# Добавить исполняемый файл
add_executable(MyAudioProject ${SOURCE_FILES})
add_custom_command(TARGET MyAudioProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "S:/C++ libraries/SFML-2.6.0-windows-vc17-64-bit/SFML-2.6.0/bin/sfml-window-2.dll"
    "S:/C++ libraries/SFML-2.6.0-windows-vc17-64-bit/SFML-2.6.0/bin/sfml-system-2.dll"
    "S:/C++ libraries/SFML-2.6.0-windows-vc17-64-bit/SFML-2.6.0/bin/sfml-graphics-2.dll"
    "S:/C++ libraries/libsndfile-1.2.2-win64/libsndfile-1.2.2-win64/bin/sndfile.dll"
    $<TARGET_FILE_DIR:MyAudioProject>)

# Связать исполняемый файл с библиотеками SFML и sndfile
target_link_libraries(MyAudioProject PRIVATE sfml-graphics sfml-window sfml-system sndfile)

#set(CMAKE_BUILD_TYPE Debug)
