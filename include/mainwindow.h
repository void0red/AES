#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "aes.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString encode(QString& plain, QString& key, QString& iv, padding_method padding, packet_method packet);
    QString decode(QString& text, QString&key, QString&iv, packet_method packet);
private:
    QString& Uint8toQString(uint8_t* str, uint32_t len);
    uint8_t* QStringtoUint8(QString& str);

private:
    Ui::MainWindow *ui;
    QString input;
    QString output;
    QString key;
    QString iv;
    padding_method padding;
    packet_method packet;

public slots:
    void EncodeButton();
    void DecodeButton();
    void ClearButton();
    void DataChanged();
};

#endif // MAINWINDOW_H
