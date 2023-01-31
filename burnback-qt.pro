QT += qml
CONFIG += \
	c++17 \
	debug_and_release

CONFIG(debug, debug|release) {
	DESTDIR = target/debug
} else {
	DESTDIR = target/release
}

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# HEADERS +=
SOURCES += \
	src/main.cpp
RESOURCES += \
	src-qml/qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
