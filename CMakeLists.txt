include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME alsoundsystem)
pr_add_library(${PROJ_NAME} SHARED)

pr_add_dependency(${PROJ_NAME} vfilesystem TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_sound TARGET)
pr_add_dependency(${PROJ_NAME} ogg TARGET)
# pr_add_external_dependency(${PROJ_NAME} steam_audio LIBRARY)
pr_add_dependency(${PROJ_NAME} vorbis TARGET)
pr_add_dependency(${PROJ_NAME} vorbisfile TARGET LINK_ONLY)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(
	${PROJ_NAME}
		-DALSYS_DLL
)

pr_finalize(${PROJ_NAME})
