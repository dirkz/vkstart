set(MODELS viking_room.obj viking_room.png)

foreach(MODEL ${MODELS})
	configure_file(${MODEL} ${MODEL} COPYONLY)
endforeach()
