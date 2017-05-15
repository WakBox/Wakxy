#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeWidgetItem>
#include <QtNetwork>
#include <QDialog>
#include <QMessageBox>

#include "packetreader.h"
#include "authserver.h"

namespace Ui {
class MainWindow;
}

enum FilterType {
    FILTER_TYPE_IS_EQUAL,
    FILTER_TYPE_IS_NOT_EQUAL,
    FILTER_TYPE_IS_SMALLER,
    FILTER_TYPE_IS_BIGGER
};

struct Packet
{
    QByteArray raw;
    PacketReader* reader;
    qint64 delayedTime;
};

typedef QList<Packet> Packets;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Log(QString line);
    void SaveCurrentSniff();
    void OpenFile(QString filename);
    void QueuePacket(Packet packet, bool isClientPacket);

    void OpenPacketDialog(QString packetType, QByteArray packet, QString script = QString());

    bool IsFiltered(ushort opcode);

public slots:
    void Open();
    void SaveAs();
    void StartProxy();
    void StopProxy();
    void StartCapture();
    void StopCapture();

    void OnNewConnection();
    void OnPacketSelect(QTreeWidgetItem* packet, int column);

    void OnClientConnect();
    void OnClientPacketRecv();
    void OnClientDisconnect();
    void OnClientError(QAbstractSocket::SocketError);

    void OnServerConnect();
    void OnServerPacketRecv();
    void OnServerDisconnect();
    void OnServerError(QAbstractSocket::SocketError);

    void AddToPacketList(PacketReader* reader, bool openFromFile = false);
    void LiveEditPacket();

    void UpdateFilter();

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    
private:
    Ui::MainWindow *ui;
    QTcpServer* m_proxy;
    bool m_started;
    bool m_capturing;
    bool m_sendPacket;
    quint32 m_packetNumber;
    qint64 m_lastPacketTime;
    Packets m_queuedPackets;

    QTcpSocket* m_client;
    quint16 m_clientPktSize;

    QTcpSocket* m_server;
    quint16 m_serverPktSize;

    bool m_fileSaved;

    AuthServer* m_auth;

    FilterType m_filterOperator;
    QList<ushort> m_filteredOpcodes;
};

#endif // MAINWINDOW_H
