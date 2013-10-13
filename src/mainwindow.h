#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QtNetwork>
#include <QDialog>
#include <QMessageBox>

#include "packetreader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Log(QString line);
    void SaveCurrentSniff();
    void AddToPacketList(PacketReader* reader);

    void OpenPacketDialog(QString packetType, QByteArray packet, QString script = QString());

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
    void OpenStructureFile();
    void OpenPacketDialog();
    
private:
    Ui::MainWindow *ui;
    QTcpServer* m_proxy;
    bool m_started;
    bool m_capturing;
    quint32 m_packetNumber;

    QTcpSocket* m_client;
    quint16 m_clientPktSize;

    QTcpSocket* m_server;
    quint16 m_serverPktSize;

    bool m_fileSaved;
};

#endif // MAINWINDOW_H
