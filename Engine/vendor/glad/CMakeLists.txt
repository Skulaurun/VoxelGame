add_library(Glad STATIC
	src/gl.c
)

target_include_directories(Glad PUBLIC include)
