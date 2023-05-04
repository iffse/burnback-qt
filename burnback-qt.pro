# QT += qml
QT += quick
QT += widgets
CONFIG += c++17

CONFIG(sanitizer) {
	message("Sanitizer enabled")
	CONFIG += sanitizer sanitize_address sanitize_undefined sanitize_leak
	QMAKE_LINK=clang++
	QMAKE_CXX = clang++
}


CONFIG(debug, debug|release) {
	DESTDIR = target/debug
} else {
	DESTDIR = target/release
}

INCLUDEPATH += include

OBJECTS_DIR = $$DESTDIR/objects
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
UI_DIR = $$DESTDIR/ui

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

HEADERS += src/headers/*.h
SOURCES += src/*.cpp
RESOURCES += src-qml/qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
