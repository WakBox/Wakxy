#ifndef DIALOGPACKET_H
#define DIALOGPACKET_H

#include <QDialog>
#include "packetreader.h"
#include "completer.h"

namespace Ui {
class DialogPacket;
}

class DialogPacket : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogPacket(QString type, QByteArray packet, QString scriptPath, QWidget *parent = 0);
    ~DialogPacket();

    void LoadFile(QString filename);
    void closeEvent(QCloseEvent* event);
    QAbstractItemModel* modelFromFile();

public slots:
    void DumpBlob();
    void Save();
    void OnTabChanged(int tab);
    void OnTextChanged();
    bool OnExit();
    
private:
    Ui::DialogPacket *ui;

    PacketReader* m_packet;
    QString m_filename;
    bool m_textChanged;

    QCompleter* m_completer;
    TextEdit* m_scriptEditor;
};

#endif // DIALOGPACKET_H
