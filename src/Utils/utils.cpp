#include "utils.h"

Utils::Utils()
{
}

QString Utils::ToHexString(QByteArray packet)
{
    QString format, hex;
    int size = packet.size();

    for(int i = 0; i < size; ++i)
        format = format % hex.sprintf("%02X ", (quint8)packet.at(i));

    if(packet.size() >= 1)
        format.chop(1); //Remove last space

    return format;
}

QByteArray Utils::FromHexString(QString packet)
{
    QString stripped = packet.remove(QRegExp("[^a-zA-Z0-9]")); //Removes everything except a-z (and capital) and 0-9

    //Error, its not an even numbered string (so not in correct form)
    if (stripped.size() % 2 != 0)
        return NULL;

    QByteArray buffer(stripped.size() % 2, '\0');
    int strippedSize = stripped.size();
    bool good;

    for (int i = 0; i < strippedSize; i += 2)
    {
        QStringRef hex(&stripped, i, 2);
        quint8 c = (quint8)hex.toString().toUShort(&good, 16);

        if(!good)
            return NULL;

        buffer.push_back(c);
    }

    return buffer;
}

QString Utils::ToASCII(QByteArray packet)
{
    int size = packet.size();
    QString ascii = QString();

    for(int i = 0; i < size; ++i)
        ascii += (isprint(packet.at(i)) ? packet.at(i) : '.');

    return ascii;
}
