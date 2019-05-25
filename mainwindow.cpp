#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSize>
#include <QCursor>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    destinationFile = new QFile;
    ui->setupUi(this);

//    scene = new QGraphicsScene();
//    view = new QGraphicsView(scene);
//    item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
//    scene->addPixmap(QPixmap(":/layout/green_mic.png"));
//    ui->graphicsView->setScene(scene);

    // Создаём палитру для тёмной темы оформления
    QPalette darkPalette;

    // Настраиваем палитру для цветовых ролей элементов интерфейса
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);

    ui->label_2->setVisible(false);
    ui->label_3->setVisible(true);

    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete destinationFile;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(true);

    ui->label_2->setVisible(true);
    ui->label_3->setVisible(false);

//    QString path = QFileDialog::getExistingDirectory(0, "Directory Dialog", "");

    QString fileName = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Save as"),
                                QDir::currentPath(),
                                "(*.wav)");

    destinationFile->setFileName(fileName);
//    QDir::setCurrent(fileName);
    destinationFile->open( QIODevice::WriteOnly | QIODevice::Truncate );

    // формат записи аудио
    // start here

    QAudioFormat format;

    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/PCM");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    //end

    setHeader(); // инициализация заголовка WAV-файла

    audio = new QAudioInput(format, this);

    QDataStream str(destinationFile);
    writeWaveHeader(wave, &str); // запись заголовка в файл

    audio->start(destinationFile);


    
}



void MainWindow::on_pushButton_2_clicked()
{
    if (destinationFile->isOpen()) {

        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);

        ui->label_2->setVisible(false);
        ui->label_3->setVisible(true);

        audio->stop();

        //доопределение заголовка WAV-файла
        //start here

        destinationFile->seek((qint64)4);
        wave.chunkSize = destinationFile->size() * 8 - 8;

        destinationFile->seek((qint64)40);
        wave.subchunk2Size = destinationFile->size() - 44;

        destinationFile->seek(0);
        QDataStream str(destinationFile);
        writeWaveHeader(wave, &str);

        //end here

        destinationFile->close();
        delete audio;


//        QFile file(fileName);
        ui->pushButton->setEnabled(true);
    }
}

void MainWindow::writeWaveHeader(WAVEHEADER header, QDataStream *Stream) {

    QDataStream::ByteOrder bo =  QDataStream::LittleEndian;
    Stream->setByteOrder(bo); // записываем данные в формате little endian

    for(int i = 0; i < sizeof(header.chunkId); i++)
        *Stream << (quint8)header.chunkId[i];

    *Stream << (quint32)header.chunkSize;

    for(int i = 0; i < sizeof(header.format); i++)
        *Stream << (quint8)header.format[i];
    for(int i = 0; i < sizeof(header.subchunk1Id); i++)
        *Stream << (quint8)header.subchunk1Id[i];

    *Stream << (quint32)header.subchunk1Size << (quint16)header.audioFormat
            << (quint16)header.numChannels << (quint32)header.sampleRate
            << (quint32)header.byteRate << (quint16)header.blockAlign
            << (quint16)header.bitsPerSample;
    for(int i = 0; i < sizeof(header.subchunk2Id); i++)
        *Stream << (quint8)header.subchunk2Id[i];

    *Stream << (quint32)header.subchunk2Size;

}

void MainWindow::setHeader() {

    destinationFile->seek(0);

    wave.chunkId[0] = 'R';
    wave.chunkId[1] = 'I';
    wave.chunkId[2] = 'F';
    wave.chunkId[3] = 'F';

    wave.chunkSize = 0;

    wave.format[0] = 'W';
    wave.format[1] = 'A';
    wave.format[2] = 'V';
    wave.format[3] = 'E';

    wave.subchunk1Id[0] = 'f';
    wave.subchunk1Id[1] = 'm';
    wave.subchunk1Id[2] = 't';
    wave.subchunk1Id[3] = ' ';

    wave.subchunk1Size = 16;
    wave.audioFormat = 1;
    wave.numChannels = 1;
    wave.sampleRate = 8000;
    wave.byteRate = wave.sampleRate * wave.numChannels * 8 / 8;
    wave.blockAlign = wave.numChannels * 8 / 8;
    wave.bitsPerSample = 8;

    wave.subchunk2Id[0] = 'd';
    wave.subchunk2Id[1] = 'a';
    wave.subchunk2Id[2] = 't';
    wave.subchunk2Id[3] = 'a';

    wave.subchunk2Size = 0;
}
