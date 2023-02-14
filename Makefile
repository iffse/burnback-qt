.DEFAULT_GOAL := run

EXECUTABLE := burnback-qt
DEBUG := target/debug
RELEASE := target/release

ifeq ($(OS),Windows_NT)
	EXECUTABLE := $(EXECUTABLE).exe
	DEBUG := $(DEBUG)/$(EXECUTABLE)
	RELEASE := $(RELEASE)/$(EXECUTABLE)
else
	UNAME_S := $(shell uname -s)
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

debug: $(DEBUG)

release: $(RELEASE)

./.qmake-debug:
	qmake -makefile -o .qmake-debug CONFIG+=debug

./.qmake-release:
	qmake -makefile -o .qmake-release CONFIG+=release

$(DEBUG): ./src ./src-qml ./.qmake-debug
	make -f .qmake-debug

$(RELEASE): ./src ./src-qml ./.qmake-release
	make -f .qmake-release

clean:
	rm -rf ./.qmake-debug ./.qmake-release ./target/ ./..qmake.stash ./.cache
