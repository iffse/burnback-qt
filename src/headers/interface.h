#pragma once

#include <QString>
#include <QDebug>

class Actions: public QObject
{
	Q_OBJECT
public:
	explicit Actions(QObject *parent = nullptr);

signals:
	void newOutput(QString output);

public slots:
	void run();
	void appendOutput(QString output);
	void worker();
};
