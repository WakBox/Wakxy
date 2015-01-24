#ifndef AUTHSERVER_H
#define AUTHSERVER_H

#include <QtNetwork>
#include "packetreader.h"

struct Config
{
    short key;
    QString value;
};

struct Realm
{
    int id;
    QString name;
    int community;
    QString ip;
    QList<int> ports;
    qint8 order;

    int versionSize;
    qint8 versionP1;
    qint16 versionP2;
    qint8 versionP3;
    QString build;

    int configSize;
    QList<Config> configs;

    int playerCount;
    int playerLimit;
    bool locked;
};

class AuthServer : public QObject
{
    Q_OBJECT

public:
    explicit AuthServer(QObject* parent = 0);
    ~AuthServer();

    QByteArray ChangeRealmList(PacketReader* reader);

public slots:
    void OnNewConnection();
    void OnClientPacketRecv();
    void OnServerPacketRecv();

private:
    QTcpServer* m_proxy;

    QTcpSocket* m_client;
    quint16 m_clientPktSize;

    QTcpSocket* m_server;
    quint16 m_serverPktSize;
};

#endif // AUTHSERVER_H
