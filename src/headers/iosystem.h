#pragma once

#include <QTextStream>

namespace Reader {
void readInput();
namespace Legacy {
void readMesh(QTextStream &in);
}

namespace Json {
void readMesh(QString &filepath);
}
}

namespace Writer {
namespace Json {
void writeData(QString &filepath, QString &origin, bool &pretty);
}
}
