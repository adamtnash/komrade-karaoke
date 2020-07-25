#ifndef SETTINGS_H
#define SETTINGS_H

#include <QVariant>

class Settings
{
public:
    static QVariant read(QString key, QString group = QString("General"));
    static void write(QString key, QVariant value, QString group = QString("General"));

    private:
        Settings();
};

#endif // SETTINGS_H
