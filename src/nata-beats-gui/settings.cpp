#include "settings.h"
#include <QSettings>

Settings::Settings()
{

}

QVariant Settings::read(QString key, QString group)
{
    QSettings settings("Nata Beats");
    settings.beginGroup(group);
    auto val = settings.value(key);
    settings.endGroup();
    return val;
}

void Settings::write(QString key, QVariant value, QString group)
{
    QSettings settings("Nata Beats");
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
}
