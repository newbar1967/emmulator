#pragma once

#include <QElapsedTimer>
#include <qDebug>

#define MEASURE_TIME(func, comment) \
do { \
        QElapsedTimer timer; \
        timer.start(); \
        func; \
        qDebug() << comment << ":" << \
        "Время выполнения:" << timer.elapsed() << "мс"; \
} while (0)
