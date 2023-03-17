.DEFAULT_GOAL := run

EXECUTABLE := burnback-qt
DEBUG := target/debug
RELEASE := target/release

ifeq ($(OS),Windows_NT)
	EXECUTABLE := $(EXECUTABLE).exe
	DEBUG := $(DEBUG)/$(EXECUTABLE)
	SANITIZER := $(SANITIZER)/$(EXECUTABLE)
	RELEASE := $(RELEASE)/$(EXECUTABLE)
	MAKE_COMMAND := nmake /F
else
	UNAME_S := $(shell uname -s)
	MAKE_COMMAND := make -f
	ifeq ($(UNAME_S),Linux)
		DEBUG := $(DEBUG)/$(EXECUTABLE)
		RELEASE := $(RELEASE)/$(EXECUTABLE)
	endif
	ifeq ($(UNAME_S),Darwin)
		DEBUG := $(DEBUG)/$(EXECUTABLE)
		RELEASE := $(RELEASE)/$(EXECUTABLE)
	endif
endif

run: $(DEBUG)
	$(DEBUG)

run-sanitizer: sanitizer
	$(DEBUG)

debug: $(DEBUG)

release: $(RELEASE)

./.qmake-debug:
	qmake -makefile -o .qmake-debug CONFIG+=debug

./.qmake-release:
	qmake -makefile -o .qmake-release CONFIG+=release

./.qmake-saint-debug:
	qmake -makefile -o .qmake-saint-debug CONFIG+=debug CONFIG+=sanitizer

$(DEBUG): ./src ./src-qml ./.qmake-debug
	$(MAKE_COMMAND) .qmake-debug

$(RELEASE): ./src ./src-qml ./.qmake-release
	$(MAKE_COMMAND) .qmake-release

sanitizer: ./src ./src-qml ./.qmake-saint-debug
	make -f .qmake-saint-debug

clean:
	rm -rf ./.qmake-debug ./.qmake-release ./target/ ./..qmake.stash ./.cache
