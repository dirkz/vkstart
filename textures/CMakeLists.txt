set(TEXTURES texture.jpg)

foreach(TEXTURE ${TEXTURES})
	configure_file(${TEXTURE} ${TEXTURE} COPYONLY)
endforeach()
