INCLUDE_DIRECTORIES(${TulipCoreBuildInclude} ${TulipCoreInclude})
FILE(GLOB LIB_SRCS "*.cpp")

ADD_LIBRARY(FastOverlapRemoval-${TulipVersion} SHARED ${LIB_SRCS})
TARGET_LINK_LIBRARIES(FastOverlapRemoval-${TulipVersion} ${LibTulipCoreName})

TULIP_INSTALL_PLUGIN(FastOverlapRemoval-${TulipVersion} ${TulipPluginsInstallDir})
