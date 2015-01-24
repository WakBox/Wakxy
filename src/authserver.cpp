#include "authserver.h"
#include "worldpacket.h"
#include "utils.h"

AuthServer::AuthServer(QObject *parent) : QObject(parent)
{
    m_proxy = new QTcpServer(this);

    if (!m_proxy->listen(QHostAddress::LocalHost, 5558))
    {
        qDebug() << m_proxy->errorString();
        return;
    }

    m_client = NULL;
    m_clientPktSize = 0;

    m_server = new QTcpSocket(this);
    m_serverPktSize = 0;

    connect(m_proxy, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
    connect(m_server, SIGNAL(readyRead()), this, SLOT(OnServerPacketRecv()));
}

AuthServer::~AuthServer()
{
    m_proxy->deleteLater();
}

void AuthServer::OnNewConnection()
{
    m_client = m_proxy->nextPendingConnection();

    if (!m_client)
        return;

    connect(m_client, SIGNAL(readyRead()), this, SLOT(OnClientPacketRecv()));

    m_server->connectToHost(QHostAddress("80.239.173.156"), 5558);
    m_server->waitForConnected(2000);
}

void AuthServer::OnClientPacketRecv()
{
    QDataStream in(m_client);

    while (m_client->bytesAvailable())
    {
        if (m_clientPktSize == 0)
        {
            if (m_client->bytesAvailable() < (qint64)sizeof(quint16))
                return;

            in >> m_clientPktSize;
        }

        if ((m_client->bytesAvailable() + 2) < m_clientPktSize)
            return;

        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);

        stream << m_clientPktSize;
        buffer += in.device()->read((qint64)(m_clientPktSize - sizeof(quint16)));

        m_server->write(buffer);
        m_clientPktSize = 0;
    }
}

void AuthServer::OnServerPacketRecv()
{
    QDataStream in(m_server);

    while (m_server->bytesAvailable())
    {
        if (m_serverPktSize == 0)
        {
            if (m_server->bytesAvailable() < (qint64)sizeof(quint16))
                return;

            in >> m_serverPktSize;
        }

        if ((m_server->bytesAvailable() + 2) < m_serverPktSize)
            return;

        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);

        stream << m_serverPktSize;
        buffer += in.device()->read((qint64)(m_serverPktSize - sizeof(quint16)));

        PacketReader* reader = new PacketReader("SMSG", buffer);
        reader->ReadHeader();

        if (reader->GetOpcode() == 1036)
        {
            QByteArray packet = ChangeRealmList(reader);
            m_client->write(packet);
            m_serverPktSize = 0;

            // Should disconnect from socket ?
            //m_server->disconnectFromHost();

            return;
        }

        m_client->write(reader->GetPacket());
        m_serverPktSize = 0;
    }
}

QByteArray AuthServer::ChangeRealmList(PacketReader* reader)
{
    QMap<int, Realm> realms;

    // Number of realms
    int realmSize = reader->ReadUInt();

    for (quint8 i = 0; i < realmSize; ++i)
    {
        Realm realm;
        realm.id = reader->ReadInt();
        realm.name = reader->ReadString(qint16(reader->ReadInt()));

        realm.community = reader->ReadInt();
        realm.ip = reader->ReadString(qint16(reader->ReadInt()));

        int portsSize = reader->ReadInt();
        for (quint8 j = 0; j < portsSize; ++j)
            realm.ports.push_back(reader->ReadInt());

        qDebug() << realm.name << " : " << realm.ports;

        realm.order = reader->ReadByte();

        realms[realm.id] = realm;
    }

    int size = reader->ReadInt();
    for (quint8 i = 0; i < size; ++i)
    {
        int serverId = reader->ReadInt();
        Realm& realm = realms[serverId];

        // Size build
        realm.versionSize = reader->ReadInt();
        {
            realm.versionP1 = reader->ReadByte();
            realm.versionP2 = reader->ReadShort();
            realm.versionP3 = reader->ReadByte();
            realm.build = reader->ReadString();
        }

        // Config size
        realm.configSize = reader->ReadInt();
        {
            int configSize = reader->ReadInt();

            for (int j = 0; j < configSize; ++j)
            {
                Config c;
                c.key = reader->ReadShort();
                c.value = reader->ReadString(reader->ReadInt());

                realm.configs.push_back(c);
            }
        }

        realm.playerCount = reader->ReadInt();
        realm.playerLimit = reader->ReadInt();
        realm.locked = reader->ReadBool();
    }

    // Test
    Realm& aerafal = realms[1];
    aerafal.name = "Aerafal Wakxy";
    aerafal.ip = "127.0.0.1";

    WorldPacket data(1036);

    data << int(realms.size());

    for (QMap<int, Realm>::ConstIterator itr = realms.begin(); itr != realms.end(); ++itr)
    {
        Realm r = (*itr);

        if (r.id == 0)
            continue;

        data << int(r.id);

        data << int(r.name.length());
        data.WriteRawBytes(r.name.toLatin1().constData(), (uint)r.name.length());

        data << int(r.community);
        data << int(r.ip.length());
        data.WriteRawBytes(r.ip.toLatin1().constData(), (uint)r.ip.length());

        data << int(r.ports.size());
        for (QList<int>::ConstIterator it = r.ports.begin(); it != r.ports.end(); ++it)
            data << int((*it));

        data << qint8(r.order);
    }

    data << int(realms.size());

    for (QMap<int, Realm>::ConstIterator itr = realms.begin(); itr != realms.end(); ++itr)
    {
        Realm r = (*itr);

        if (r.id == 0)
            continue;

        data << int(r.id);

        data << int(r.versionSize);
        data << qint8(r.versionP1);
        data << (qint16)r.versionP2;
        data << (qint8)r.versionP3;

        data << quint8(r.build.length());
        data.WriteRawBytes(r.build.toLatin1().constData(), (uint)r.build.length());

        data << (int)r.configSize;
        data << (int)r.configs.size();

        for (QList<Config>::ConstIterator it = r.configs.begin(); it != r.configs.end(); ++it)
        {
            data << (*it).key;

            QString value = (*it).value;
            data << int(value.length());
            data.WriteRawBytes(value.toLatin1().constData(), (uint)value.length());
        }

        data << (int)r.playerCount;
        data << (int)r.playerLimit;
        data << (qint8)r.locked;
    }

    data.WriteHeader();
    return data.GetPacket();
}
