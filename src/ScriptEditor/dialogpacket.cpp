#include "dialogpacket.h"
#include "ui_dialogpacket.h"
#include <QMessageBox>
#include <QStringListModel>
#include "highlighter.h"

DialogPacket::DialogPacket(QString type, QByteArray packet, QString scriptPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPacket)
{
    ui->setupUi(this);

    m_packet = NULL;
    m_filename = QString();
    m_textChanged = false;

    m_scriptEditor = new TextEdit(this);
    m_scriptEditor->setGeometry(5, 5, 725, 545);
    m_completer = new QCompleter(this);
    m_completer->setModel(modelFromFile());
    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);
    m_scriptEditor->setCompleter(m_completer);

    QFont font;
    font.setFamily("Myriad");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);

    QFontMetrics metrics(font);
    Highlighter* highlighter = new Highlighter(m_scriptEditor->document());

    m_scriptEditor->setFont(font);
    m_scriptEditor->setTabStopWidth(4 * metrics.width(" "));
    ui->tabWidget->removeTab(0);
    ui->tabWidget->insertTab(0, m_scriptEditor, tr("Script Editor"));
    ui->tabWidget->setCurrentIndex(0);
    m_scriptEditor->setFocus();

    if (!packet.isEmpty())
    {
        m_packet = new PacketReader(type, packet);
        m_packet->ReadHeader();

        ui->Opcode->setText(QString::number(m_packet->GetOpcode()));
        ui->Opcode->setDisabled(true);
        ui->Type->setCurrentIndex((m_packet->GetType() == "CMSG") ? 0 : 1);
        ui->Type->setDisabled(true);
        ui->Size->setText(QString::number(m_packet->GetSize()));

        if (m_packet->CompileScript())
            ui->tabWidget->setCurrentIndex(1);

        ui->ParsedPacket->setPlainText(m_packet->GetAnalyzedPacket());
    }

    if (m_packet && !m_packet->GetScript().isEmpty())
        m_scriptEditor->setPlainText(m_packet->GetScript());
    else if (!scriptPath.isEmpty())
        LoadFile(scriptPath);
    else
        m_scriptEditor->setPlainText("function ReadPacket()\n{\n\t// Code\n}\n\nReadPacket();");

    connect(ui->Blob, SIGNAL(clicked()), this, SLOT(DumpBlob()));
    connect(ui->Close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->Save, SIGNAL(clicked()), this, SLOT(Save()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
}

DialogPacket::~DialogPacket()
{
    delete ui;
}

void DialogPacket::LoadFile(QString filename)
{
    QStringList path = filename.split("/");
    int s = path.size();

    ui->Opcode->setText(path[s - 1].split(".").at(0));
    ui->Type->setCurrentIndex((path[s - 2] == "Client") ? 0 : 1);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString script = in.readAll();

    m_scriptEditor->setPlainText(script);
}

QAbstractItemModel* DialogPacket::modelFromFile()
{
    QStringList words;
    words << "packet" << "Comment(\"comment\");" << "Log(\"message\");" << "Lengt();"
            << "ReadBool(\"name\");"
            << "ReadInt(\"name\");"
            << "ReadUInt(\"name\");"
            << "ReadFloat(\"name\");"
            << "ReadDouble(\"name\");"
            << "ReadShort(\"name\");"
            << "ReadUShort(\"name\");"
            << "ReadByte(\"name\");"
            << "ReadUByte(\"name\");"
            << "ReadLong(\"name\");"
            << "ReadULong(\"name\");"
            << "QString ReadString(\"name\");";

    return new QStringListModel(words, m_completer);
}

void DialogPacket::DumpBlob()
{
    if (!m_packet)
        return;

    QDir dir(QCoreApplication::applicationDirPath());

    #ifdef Q_OS_MAC
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
    #endif

    QString path = dir.absolutePath() + "/Dumps/" + QString::number(m_packet->GetOpcode()) + ".bin";

    QFile blob(path);
    if (!blob.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Error !"), tr("The file %1 can't be written !").arg(path));
        return;
    }

    QByteArray data = m_packet->GetPacket();
    data.remove(0, 8);

    blob.write(data);
    blob.close();
}

void DialogPacket::Save()
{
    QString filename = m_filename;

    if (filename.isEmpty())
    {
        QDir dir(QCoreApplication::applicationDirPath());

        #ifdef Q_OS_MAC
            dir.cdUp();
            dir.cdUp();
            dir.cdUp();
        #endif

        QString path = dir.absolutePath() + "/Packets/";
        QString opcode, type;

        if (!m_packet)
        {
            opcode = ui->Opcode->text();
            type = (ui->Type->currentText() == "CMSG") ? "Client/" : "Server/";
        }
        else
        {
            opcode = QString::number(m_packet->GetOpcode());
            type = (m_packet->GetType() == "CMSG") ? "Client/" : "Server/";
        }

        filename = path + type + opcode + ".js";
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error !"), tr("The file %1 can't be written !").arg(filename));
        return;
    }

    QTextStream out(&file);
    out << m_scriptEditor->toPlainText();

    file.close();
    m_textChanged = false;

    if (m_filename.isEmpty())
    {
        QMessageBox::information(this, tr("Script file saved !"), tr("The file was successfully saved."), QMessageBox::Ok);
        m_filename = filename;
    }
}

void DialogPacket::OnTabChanged(int tab)
{
    if (tab == 1)
    {
        if (m_packet && m_packet->CompileScript(m_scriptEditor->toPlainText()))
            ui->ParsedPacket->setPlainText(m_packet->GetAnalyzedPacket());
    }
}

void DialogPacket::OnTextChanged()
{
    if (m_scriptEditor->toPlainText() == "" && m_filename.isEmpty())
        return;

    m_textChanged = true;
}

void DialogPacket::closeEvent(QCloseEvent* event)
{
    if (OnExit())
        event->accept();
    else
        event->ignore();
}

bool DialogPacket::OnExit()
{
    if (m_textChanged)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The script file has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int result = msgBox.exec();

        if (result == QMessageBox::Save)
            Save();
        else if (result == QMessageBox::Cancel)
            return false;
    }

    this->close();
    return true;
}
