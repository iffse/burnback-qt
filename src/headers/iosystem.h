#pragma once

#include <QTextStream>

namespace Reader {
void readInput();
namespace Legacy {
void readMesh(QTextStream &in);
}

namespace Json {
void readMesh(QString &filepath);;
}
}

namespace Writer {
	void writeiData();
	void writerData();
	void outResult();
}
