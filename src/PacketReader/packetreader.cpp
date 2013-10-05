#include "packetreader.h"
#include <QMessageBox>

PacketReader::PacketReader(QString type, QByteArray packet, QObject* parent) : QObject(parent)
{
    m_packet    = packet;
    m_packetStream = new QDataStream(m_packet);

    m_analyzedPacket = QString();
    m_analyzedPacketStream = new QTextStream(&m_analyzedPacket);

    m_type      = type;
    m_size      = 0;
    m_opcode    = 0;

    m_scriptFilename = QString();
    m_script = QString();
    m_scriptEngine = NULL;
}

void PacketReader::ReadHeader()
{
    *m_packetStream >> m_size;

    if (m_type == "CMSG")
    {
        qint8 unk1;
        *m_packetStream >> unk1;
    }

    *m_packetStream >> m_opcode;
}

bool PacketReader::ScriptFileExist()
{
    m_scriptFilename += "Packets/";
    m_scriptFilename += (GetType() == "CMSG") ? "Client/" : "Server/";
    m_scriptFilename += QString::number(GetOpcode()) + ".js";

    return QFile::exists(m_scriptFilename);
}

bool PacketReader::CompileScript(QString script)
{
    m_analyzedPacket = QString();
    m_analyzedPacketStream->seek(0);
    m_packetStream->device()->seek(0);

    ReadHeader();

    if (m_scriptFilename.isEmpty())
        if (!ScriptFileExist())
            return false;

    if (!m_scriptEngine)
    {
        m_scriptEngine = new QScriptEngine(this);
        m_scriptEngine->globalObject().setProperty("packet", m_scriptEngine->newQObject(this));
    }

    *m_analyzedPacketStream << "Opcode : " << GetOpcode() << "\r\n";
    *m_analyzedPacketStream << "Type : " << GetType() << "\r\n";
    *m_analyzedPacketStream << "Size : " << GetSize() << "\r\n\r\n";
    *m_analyzedPacketStream << "Structure : \r\n\r\n";

    if (script.isEmpty())
    {
        QFile file(m_scriptFilename);

        if (!file.open(QIODevice::ReadOnly))
        {
            file.errorString();
            return false;
        }

        QTextStream in(&file);
        script = in.readAll();
        file.close();

        m_script = script;
    }

    QScriptValue result = m_scriptEngine->evaluate(script);
    if (result.isError())
    {
        QMessageBox::critical(0, tr("Script error"), QString::fromLatin1("%0:%1: %2").arg(m_scriptFilename).arg(result.property("lineNumber").toInt32()).arg(result.toString()));
        return false;
    }

    return true;
}

void PacketReader::Log(QVariant name)
{
    *m_analyzedPacketStream << "[LOG] " << name.toString() << "\n\n";
}

int PacketReader::Length()
{
    return (int)m_packetStream->device()->bytesAvailable();
}

void PacketReader::Comment(QString comment)
{
    *m_analyzedPacketStream << "// " << comment << "\n";
}

template <typename T> T PacketReader::Read(QString name)
{
    T v;
    *m_packetStream >> v;
    *m_analyzedPacketStream << name << " : " << v << "\n\n";
    return v;
}

bool PacketReader::ReadBool(QString name) { return Read<bool>(name + " [Bool]"); }
qint32 PacketReader::ReadInt(QString name) { return Read<qint32>(name + " [Int]"); }
quint32 PacketReader::ReadUInt(QString name) { return Read<quint32>(name + " [UInt]"); }
float PacketReader::ReadFloat(QString name) { return Read<float>(name + " [Float]"); }
double PacketReader::ReadDouble(QString name) { return Read<double>(name + " [Double]"); }
qint16 PacketReader::ReadShort(QString name) { return Read<qint16>(name + " [Short]"); }
quint16 PacketReader::ReadUShort(QString name) { return Read<quint16>(name + " [UShort]"); }
char PacketReader::ReadByte(QString name){ return Read<qint8>(name + " [Byte]"); }
uchar PacketReader::ReadUByte(QString name) { return Read<quint8>(name + " [UByte]"); }
qint64 PacketReader::ReadLong(QString name) { return Read<qint64>(name + " [Long]"); }
quint64 PacketReader::ReadULong(QString name) { return Read<quint64>(name + " [ULong]"); }
QString PacketReader::ReadString(QString name) { return Read<QString>(name + " [QString]"); }
