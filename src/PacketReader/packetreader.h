#ifndef PACKETREADER_H
#define PACKETREADER_H

#include <QtCore>
#include <QObject>
#include <QtScript>

class PacketReader : public QObject
{
    Q_OBJECT

public:
    explicit PacketReader(QString type, QByteArray packet, QObject* parent = 0);

    void ReadHeader();

    bool ScriptFileExist();
    bool CompileScript(QString script = QString());

    void SetPacket(QByteArray packet) { m_packet = packet; }
    QByteArray GetPacket() { return m_packet; }
    QString GetType() { return m_type; }
    ushort GetSize() { return m_size; }
    ushort GetOpcode() { return m_opcode; }
    QString GetScriptFilename() { return m_scriptFilename; }
    QString GetScript() { return m_script; }

    QString GetAnalyzedPacket()
    {
        return m_analyzedPacketStream->readAll();
    }

    template <typename T> T Read(QString name = QString());
    qint8 ReadInt8();
    quint8 ReadUInt8();

public slots:
    void Log(QVariant name);
    int Length();
    void Comment(QString comment);

    bool ReadBool(QString name = QString());
    qint32 ReadInt(QString name = QString());
    quint32 ReadUInt(QString name = QString());
    float ReadFloat(QString name = QString());
    double ReadDouble(QString name = QString());
    qint16 ReadShort(QString name = QString());
    quint16 ReadUShort(QString name = QString());
    char ReadByte(QString name = QString());
    uchar ReadUByte(QString name = QString());
    qint64 ReadLong(QString name = QString());
    quint64 ReadULong(QString name = QString());
    QString ReadString(quint16 length, QString name = QString());
    QString ReadString(QString name = QString());
    QString ReadBigString(QString name = QString());
    void Skip(quint16 length);
    void DumpBlob(QString filename, quint32 size = 0);

protected:
    QByteArray m_packet;
    QDataStream* m_packetStream;

    QString m_analyzedPacket;
    QTextStream* m_analyzedPacketStream;

    QString m_type;
    ushort m_size;
    ushort m_opcode;

    QString m_scriptFilename;
    QString m_script;
    QScriptEngine* m_scriptEngine;
};

#endif // PACKETREADER_H
