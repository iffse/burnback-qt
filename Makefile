.DEFAULT_GOAL := run

run: ./target/debug/burnback-qt
	./target/debug/burnback-qt

debug: ./target/debug/burnback-qt

release: ./target/release/burnback-qt

./.qmake-debug:
	qmake -makefile -o .qmake-debug CONFIG+=debug

./.qmake-release:
	qmake -makefile -o .qmake-release CONFIG+=release

./target/debug/burnback-qt: ./src ./src-qml ./.qmake-debug
	make -f .qmake-debug

./target/release/burnback-qt: ./src ./src-qml ./.qmake-release
	make -f .qmake-release

clean:
	rm -rf ./.qmake-debug ./.qmake-release ./target/ ./..qmake.stash ./.cache
