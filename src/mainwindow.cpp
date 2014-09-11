#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "dialogpacket.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_proxy = new QTcpServer(this);
    m_started = false;
    m_capturing = false;
    m_sendPacket = true;
    m_packetNumber = 0;
    m_lastPacketTime = 0;
    m_queuedPackets.clear();

    m_client = NULL;
    m_clientPktSize = 0;

    m_server = new QTcpSocket(this);
    m_serverPktSize = 0;

    connect(m_server, SIGNAL(connected()), this, SLOT(OnServerConnect()));
    connect(m_server, SIGNAL(readyRead()), this, SLOT(OnServerPacketRecv()));
    connect(m_server, SIGNAL(disconnected()), this, SLOT(OnServerDisconnect()));
    connect(m_server, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnServerError(QAbstractSocket::SocketError)));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(ui->buttonStartProxy, SIGNAL(clicked()), this, SLOT(StartProxy()));
    connect(ui->buttonStartCapture, SIGNAL(clicked()), this, SLOT(StartCapture()));
    connect(ui->buttonLiveEdit, SIGNAL(clicked()), this, SLOT(LiveEditPacket()));
    connect(ui->packets, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(OnPacketSelect(QTreeWidgetItem*,int)));
    connect(m_proxy, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

    QDir dir;
    dir.mkdir("Packets");
    dir.mkpath("Packets/Server");
    dir.mkpath("Packets/Client");

    m_fileSaved = false;
}

MainWindow::~MainWindow()
{
    StopProxy();
    m_proxy->deleteLater();

    delete ui;
}

void MainWindow::Log(QString line)
{
    ui->logs->appendPlainText(">> " + line);
}

void MainWindow::SaveCurrentSniff()
{
    if (ui->packets->topLevelItemCount() > 0 && !m_fileSaved)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Current sniff are not saved."));
        msgBox.setInformativeText(tr("Do you want to save it ? Otherwise it'll be deleted."));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int result = msgBox.exec();

        if (result == QMessageBox::Yes)
            SaveAs();
    }

    ui->packets->clear();
    m_packetNumber = 0;
}

void MainWindow::Open()
{
    SaveCurrentSniff();

    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Wakxy file (*.wxy)"));

    if (filename.isNull())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QStringList line = in.readLine().split(";");
        PacketReader reader(line.at(0), Utils::FromHexString(line.at(1)));
        reader.ReadHeader();

        AddToPacketList(&reader, true);
    }

    m_fileSaved = true;
}

void MainWindow::SaveAs()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save as..."), "", tr("Wakxy file (*.wxy)"));

    if (filename.isNull())
        return;

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error !"), tr("The file %1 can't be written !").arg(filename));
        return;
    }

    QTextStream out(&file);
    QTreeWidgetItemIterator itr(ui->packets);

    while (*itr)
    {
        out << (*itr)->text(1) << ";" << (*itr)->text(5) << "\n";
        ++itr;
    }

    file.close();
    m_fileSaved = true;
}

void MainWindow::StartProxy()
{
    if (m_started)
        return StopProxy();

    if (!m_proxy->listen(QHostAddress::LocalHost, 4443))
    {
        qDebug() << m_proxy->errorString();
        return;
    }

    SaveCurrentSniff();

    m_fileSaved = false;
    m_started = true;
    ui->buttonStartProxy->setText(tr("Stop proxy"));

    ui->logs->clear();

    Log(tr("Proxy started ! You can now login."));
}

void MainWindow::StopProxy()
{
    if (!m_started)
        return;

    m_started = false;

    if (m_client)
    {
        m_client->abort();
        m_client->deleteLater();
    }

    m_server->abort();
    m_proxy->close();

    ui->buttonStartProxy->setText(tr("Start proxy"));
    Log(tr("Proxy stopped."));
}

void MainWindow::StartCapture()
{
    if (m_capturing)
        return StopCapture();

    SaveCurrentSniff();

    m_capturing = true;
    ui->buttonStartCapture->setText(tr("Stop capturing"));
    Log(tr("Packet capture started."));
}

void MainWindow::StopCapture()
{
    m_capturing = false;
    ui->buttonStartCapture->setText(tr("Start capturing"));
    Log(tr("Packet capture stopped."));
}

void MainWindow::OnNewConnection()
{
    m_client = m_proxy->nextPendingConnection();

    if (!m_client)
        return;

    connect(m_client, SIGNAL(connected()), this, SLOT(OnClientConnect()));
    connect(m_client, SIGNAL(readyRead()), this, SLOT(OnClientPacketRecv()));
    connect(m_client, SIGNAL(disconnected()), this, SLOT(OnClientDisconnect()));
    connect(m_client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnClientError(QAbstractSocket::SocketError)));

    m_server->connectToHost(QHostAddress("127.0.0.1"), 4444);
    //m_server->connectToHost(QHostAddress("80.239.173.153"), 443);
    m_server->waitForConnected(2000);
}

void MainWindow::OnClientConnect()
{
    Log(tr("Client connect to proxy from ") + m_client->localAddress().toString() + ":" + QString::number(m_client->localPort()));
}

void MainWindow::OnClientPacketRecv()
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

        PacketReader* reader = new PacketReader("CMSG", buffer);
        reader->ReadHeader();

        Packet packet;
        packet.raw = buffer;
        packet.reader = reader;
        packet.delayedTime = 0;

        QueuePacket(packet, true);
        m_clientPktSize = 0;
    }
}

void MainWindow::OnClientDisconnect()
{
    Log(tr("Client disconnected from proxy."));
    StopProxy();
}

void MainWindow::OnClientError(QAbstractSocket::SocketError /*error*/)
{
    Log(tr("Error with client socket. Stopping Wakxy..."));
    Log(tr("Error log : ") + m_client->errorString());
    StopProxy();
}

void MainWindow::OnServerConnect()
{
    Log(tr("Proxy successfully connect to Wakfu server (") + m_server->peerAddress().toString() + ":" + QString::number(m_server->peerPort()) + ")");
}

void MainWindow::OnServerPacketRecv()
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

        Packet packet;
        packet.raw = buffer;
        packet.reader = reader;
        packet.delayedTime = 0;

        QueuePacket(packet, false);
        m_serverPktSize = 0;
    }
}

void MainWindow::OnServerDisconnect()
{
    Log(tr("Proxy disconnected from Wakfu server."));
    StopProxy();
}

void MainWindow::OnServerError(QAbstractSocket::SocketError /*error*/)
{
    Log(tr("Error when proxy trying to connect to Wakfu server."));
    Log(tr("Error log : ") + m_server->errorString());
    StopProxy();
}

void MainWindow::QueuePacket(Packet packet, bool isClientPacket)
{
    if (m_sendPacket)
    {
        qDebug() << "Direct send packet";
        (isClientPacket) ? m_server->write(packet.raw) : m_client->write(packet.raw);
    }
    else
    {
        qDebug() << "Queue packet";
        packet.delayedTime = (m_lastPacketTime > 0) ? QDateTime::currentMSecsSinceEpoch() - m_lastPacketTime : 0;
        m_queuedPackets.push_back(packet);
    }

    m_lastPacketTime = QDateTime::currentMSecsSinceEpoch();
    AddToPacketList(packet.reader);
}

void MainWindow::AddToPacketList(PacketReader* reader, bool openFromFile)
{
    if (!m_capturing && !openFromFile)
        return;

    if ((reader->GetOpcode() == 1024 || reader->GetOpcode() == 1025) && !ui->logLoginPacket->isChecked())
        return;

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, QString::number(m_packetNumber));
    item->setText(1, reader->GetType());
    item->setText(2, QString::number(reader->GetSize()));
    item->setText(3, QString::number(reader->GetOpcode()));
    item->setText(4, Utils::ToASCII(reader->GetPacket()));
    item->setText(5, Utils::ToHexString(reader->GetPacket()));
    ui->packets->insertTopLevelItem(m_packetNumber++, item);

    if (m_sendPacket && !openFromFile)
        delete reader;
}

void MainWindow::OnPacketSelect(QTreeWidgetItem* packet, int /*column*/)
{
    OpenPacketDialog(packet->text(1), Utils::FromHexString(packet->text(5)));
}

void MainWindow::OpenPacketDialog(QString packetType, QByteArray packet, QString script)
{
    DialogPacket dialog(packetType, packet, script, this);
    dialog.exec();
}

void MainWindow::LiveEditPacket()
{
    if (m_sendPacket)
    {
        m_sendPacket = false;
        m_capturing = true;
    }
    else
    {
        m_sendPacket = true;
        while (!m_queuedPackets.isEmpty())
        {
            Packet packet = m_queuedPackets.takeFirst();

            QEventLoop loop;
            QTimer timer;

            timer.setSingleShot(true);
            timer.setInterval(packet.delayedTime);
            qDebug() << packet.delayedTime;

            connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            timer.start();
            loop.exec();

            qDebug() << packet.reader->GetPacket();
            qDebug() << packet.reader->GetType();
            (packet.reader->GetType() == "CMSG") ? m_server->write(packet.raw) : m_client->write(packet.raw);
            qDebug() << "Queued packet sended !";
            delete packet.reader;
        }
    }
}
