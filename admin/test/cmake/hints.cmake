# cmake/hints.cmake

# Qt/Qwt:
set(CMAKE_PREFIX_PATH "/opt/qt5;/opt/qwt-6.1.4" CACHE STRING "")

# Qwt (explicit hints if no QwtConfig.cmake is present)
set(QWT_INCLUDE_DIR "/opt/qwt-6.1.4/include" CACHE PATH "")
set(QWT_LIBRARY     "/opt/qwt-6.1.4/lib/libqwt.so" CACHE FILEPATH "")

# Examples for other libs:
# set(OPENSSL_ROOT_DIR "/opt/openssl" CACHE PATH "")
# set(MYSQL_INCLUDE_DIR "/opt/mysql/include" CACHE PATH "")
# set(MYSQL_LIBRARY     "/opt/mysql/lib/libmysqlclient.so" CACHE FILEPATH "")
# set(LibArchive_INCLUDE_DIR "/opt/libarchive/include" CACHE PATH "")
# set(LibArchive_LIBRARY     "/opt/libarchive/lib/libarchive.so" CACHE FILEPATH "")

# Optional: help pkg-config
set(ENV{PKG_CONFIG_PATH} "/opt/qt5/lib/pkgconfig:/opt/qwt-6.1.4/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
