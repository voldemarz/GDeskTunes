#include "cookiejar.h"
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtNetwork/QNetworkCookie>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QDesktopServices>
#else
#include <QtCore/QStandardPaths>
#endif

static const unsigned int JAR_VERSION = 01;

QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for(quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
}

void CookieJar::load()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    qDebug() << "Cookies from " << directory;
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
}

void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);
}

void CookieJar::removeCookieFile()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    cookieSettings.clear();
}

void CookieJar::deleteAllCookies()
{
    QList<QNetworkCookie> empty_list;
    setAllCookies(empty_list);
    removeCookieFile();
}


void CookieJar::save()
{
    purgeOldCookies();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    qDebug() << "Saving cookies in " << directory;
    if (!QFile::exists(directory))
    {
        QDir dir;
        dir.mkpath(directory);
    }
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for(int i=cookies.count()-1; i>=0; --i)
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    cookieSettings.setValue(QLatin1String("cookies"), QVariant::fromValue<QList<QNetworkCookie> >(cookies));
}


