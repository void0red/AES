#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(800, 391);


    connect(ui->encode_button, SIGNAL(clicked()), this, SLOT(EncodeButton()));
    connect(ui->decode_button, SIGNAL(clicked()), this, SLOT(DecodeButton()));
    connect(ui->clear_button, SIGNAL(clicked()), this, SLOT(ClearButton()));

    connect(ui->input, SIGNAL(textChanged()), this, SLOT(DataChanged()));
    connect(ui->key, SIGNAL(textChanged()), this, SLOT(DataChanged()));
    connect(ui->iv, SIGNAL(textChanged()), this, SLOT(DataChanged()));

    connect(ui->padding_box, SIGNAL(activated(int)), this, SLOT(DataChanged()));
    connect(ui->packet_box, SIGNAL(activated(int)), this, SLOT(DataChanged()));

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::EncodeButton()
{
    if(input.length() <= 0|| key.length() <= 0)
        return;
    if(key.length() < 16){
        QMessageBox::critical(this, "Warning", "key的长度必须是16字节", QMessageBox::Yes);
        return;
    }
    output = encode(input, key, iv, padding, packet);
    auto hexout = QString(output);
    ui->output->setPlainText(hexout);
}
void MainWindow::DecodeButton()
{
    if(input.length() <= 0|| key.length() <= 0)
        return;
    if(key.length() < 16){
        QMessageBox::critical(this, "Warning", "key的长度必须是16字节", QMessageBox::Yes);
        return;
    }
    output = decode(input, key, iv, packet);
    auto hexout = QString(output);
    ui->output->setPlainText(hexout);
}
void MainWindow::ClearButton(){
    ui->input->setPlainText("");
    ui->output->setPlainText("");
}
void MainWindow::DataChanged(){
    input = ui->input->toPlainText();
    key = ui->key->toPlainText();
    iv = ui->iv->toPlainText();

    padding = static_cast<padding_method>(ui->padding_box->currentIndex()+1);
    packet = static_cast<packet_method>(ui->packet_box->currentIndex());

    if(packet == ECB) {
        ui->iv->setVisible(false);
        ui->iv_label->setVisible(false);
    }else {
        ui->iv->setVisible(true);
        ui->iv_label->setVisible(true);
    }
}
QString MainWindow::encode(QString &plain, QString &key, QString &iv, padding_method padding, packet_method packet)
{
    uint8_t* _in = QStringtoUint8(plain);
    uint8_t* _key = QStringtoUint8(key);
    uint8_t* _iv = QStringtoUint8(iv);
    uint32_t out_size;
    uint8_t* out = aes_encode(_in, plain.length(), _key, key.length(), &out_size, _iv, padding, packet);

    return Uint8toQString(out, out_size);
}
QString MainWindow::decode(QString &text, QString &key, QString &iv, packet_method packet)
{
    uint8_t* _in = QStringtoUint8(text);
    uint8_t* _key = QStringtoUint8(key);
    uint8_t* _iv = QStringtoUint8(iv);
    uint32_t out_size;

    uint8_t* out = aes_decode(_in, text.length(), _key, key.length(), &out_size, _iv, packet);

    return Uint8toQString(out, out_size);
}
QString& MainWindow::Uint8toQString(uint8_t* str, uint32_t len)
{
    QByteArray qb(reinterpret_cast<char*>(str), static_cast<int>(len));
    auto ret = new QString(qb.toHex());
    return *ret;
}
uint8_t* MainWindow::QStringtoUint8(QString& str)
{
    auto qb = new QByteArray(str.toLatin1());
    auto ptr = qb->data();
    auto ret = reinterpret_cast<uint8_t*>(ptr);
    return ret;
}
