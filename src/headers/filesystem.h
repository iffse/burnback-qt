#pragma once

#include <QTextStream>

namespace Reader {
	void readInput();
	void readMesh(QTextStream &in);
	void inResult();
}

namespace Writer {
	void writeiData();
	void writerData();
	void outResult();
}
