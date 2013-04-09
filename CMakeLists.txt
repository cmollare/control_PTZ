cmake_minimum_required(VERSION 2.6) # Pour éviter un warning au lancement de CMake, on doit préciser la version minimale

# Nom du projet
project(control_PTZ)

include_directories(
	/usr/include/opencv2
	#./dep/include/opencv
	#./dep/include/
	/usr/include/
)

link_directories(
	#../dep/lib
	/usr/lib
)

find_package (Threads)

SET(EXECUTABLE_OUTPUT_PATH ../bin/)


file(
    GLOB_RECURSE
    source_files
    src/*
)

# Déclaration de l'exécutable
add_executable(
    control_PTZ
    ${source_files}
)

target_link_libraries(
        control_PTZ
#libs de OpenCV
	opencv_core
	opencv_highgui
	opencv_imgproc
	opencv_video
	opencv_legacy
	opencv_contrib
#libs de curl
	curl
	${CMAKE_THREAD_LIBS_INIT}
)


