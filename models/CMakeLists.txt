set(MODELS viking_room.obj viking_room.png)

foreach(MODELS ${MODEL})
	configure_file(${MODEL} ${MODEL} COPYONLY)
endforeach()
