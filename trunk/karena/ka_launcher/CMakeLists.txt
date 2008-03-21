if(ENABLE_LAUNCHER)

	find_package(wxWidgets REQUIRED)
	add_definitions(${wxWidgets_DEFINITIONS})
	include_directories(${wxWidgets_INCLUDE_DIRS})

	add_executable(kalaunch
		kalaunch.cpp
	)
	target_link_libraries(kalaunch ${wxWidgets_LIBRARIES})

	install(TARGETS kalaunch DESTINATION ${BINDIR})

#EXTRA_DIST = \
#	karena.xpm \
#	kalaunch.ico \
#	kalaunch.rc \
#	kalaunch.sln \
#	kalaunch.vcproj \
#	kalaunch.xpm

endif(ENABLE_LAUNCHER)
