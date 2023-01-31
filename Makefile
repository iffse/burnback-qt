.DEFAULT_GOAL := run

run: ./target/debug/burnback-qt
	./target/debug/burnback-qt

debug: ./target/debug/burnback-qt

release: ./target/release/burnback-qt

qmakefile:
	qmake -makefile -o qmakefile

./target/debug/burnback-qt: ./src ./src-qml qmakefile
	make -f qmakefile debug

./target/release/burnback-qt: ./src ./src-qml qmakefile
	make -f qmakefile release

clean:
	rm -rf ./debug ./release/ ./qmakefile* ./target/ ./.qmake.stash ./.cache
