PROJECT (LIBhfs_iso C)
INCLUDE_DIRECTORIES(../include ../libhfs_iso ../wodim ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/include)
ADD_DEFINITIONS(-DUSE_LARGEFILES -DABORT_DEEP_ISO_ONLY -DAPPLE_HYB -DUDF -DDVD_VIDEO -DSORTING -DHAVE_CONFIG_H -DUSE_LIBSCHILY -DUSE_SCG)


SET(LIBhfs_iso_SRCS block.c btree.c data.c file.c gdata.c hfs.c low.c node.c record.c volume.c)

ADD_LIBRARY (hfs_iso STATIC ${LIBhfs_iso_SRCS})
