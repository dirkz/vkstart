set(SHADER_SOURCES shader.slang)

add_custom_target(shaders)

function(create_shader source)
	set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${source})
	set(SHADER_OUTPUT_FILENAME ${source}.spv)
	set(SHADER_TARGET ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_OUTPUT_FILENAME})

	set(OPTIONS1 -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name)
	set(ENTRIES -entry VertMain -entry FragMain)

	add_custom_command(
		OUTPUT ${SHADER_TARGET}
		DEPENDS ${source}
		COMMAND slangc ${SHADER_SRC} ${OPTIONS1} ${ENTRIES} -o ${SHADER_TARGET}
	)

	target_sources(shaders PRIVATE ${SHADER_TARGET})
endfunction()

foreach(SHADER ${SHADER_SOURCES})
	create_shader(${SHADER})
endforeach()
