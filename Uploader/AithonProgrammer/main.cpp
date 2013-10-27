#include <QtCore/QCoreApplication>
#include <QDebug>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QFile>
#include <QTime>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <unistd.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#define START_ADDR  0x08008000
#define PACKET_LEN  1024

QextSerialPort *port;

void error(QString msg)
{
    qDebug() << msg;
    exit(1);
}

bool waitAsk(QString msg)
{
    while (!port->bytesAvailable());
    uint8_t ask = port->read(1).at(0);
    if (ask == 0x79)
    {
        // ACK
        // do nothing
    }
    else if (ask == 0x1F)
    {
        // NACK
        //qDebug() << "NACK " << msg;
        return false;
    }
    else
    {
        // Unknown response
        error("Unknown response " + QString::number((int)ask, 16) + ": " + msg);
    }
    return true;
}

void writeByte(uint8_t byte)
{
    port->write((const char *)&byte, 1);
}

void initChip()
{
    writeByte(0x11);
#ifdef Q_OS_WIN32
    Sleep(uint(500));
#else
    sleep(uint(500));
#endif
    writeByte(0x11);
#ifdef Q_OS_WIN32
    Sleep(uint(100));
#else
    sleep(uint(100));
#endif
    writeByte(0x7F);
    waitAsk("Syncro");
}

void writeWithAsk(uint8_t cmd)
{
    writeByte(cmd);
    waitAsk(QString::number(cmd, 16));
}

void sendAddress(uint32_t addr)
{
    writeByte((addr >> 24) & 0xFF);
    writeByte((addr >> 16) & 0xFF);
    writeByte((addr >> 8) & 0xFF);
    writeByte((addr >> 0) & 0xFF);
    waitAsk("0x31 address failed");
}

void startProgram()
{
    std::cout << "Starting program...\t\t";
    writeWithAsk(0x21);
    std::cout << "Done\n";
}

void writeMemoryAddress(uint32_t addr, QByteArray data)
{
    if (data.length() > PACKET_LEN)
    {
        qDebug() << data.length();
        error("Data too long!");
    }
    // send address
    writeWithAsk(0x31);
    sendAddress(addr);
    // send length
    QByteArray dataBytes;
    uint16_t len = (data.length() - 1) & 0xFFFF;
    dataBytes.append((uint8_t) (len >> 8));
    dataBytes.append((uint8_t) (len & 0xFF));
    // send data
    uint8_t crc = 0xFF;
    for (int i = 0; i < data.length(); i++)
    {
        uint8_t c = data.at(i);
        crc = crc ^ c;
        dataBytes.append(c);
    }
    port->write(dataBytes);
    writeByte(crc);
    if (!waitAsk("Program data"))
    {
        qDebug() << "RETRY";
        return writeMemoryAddress(addr, data);
    }
}

void eraseMemory()
{
    writeWithAsk(0x43);
    std::cout << "Erasing flash...\t\t";
    waitAsk("0x43 erasing failed");
    std::cout << "Done\n";
}

QByteArray getChunk(QByteArray data, int start, int len)
{
    QByteArray result = QByteArray();
    for (int i = start; i < start+len; i++)
    {
        result.append(data.at(i));
    }
    return result;
}

void writeMemory(uint32_t addr, QByteArray data)
{
    int len = data.length();
    std::cout << "Writing program data...\t\t";
    int offs = 0;
    while (len > PACKET_LEN)
    {
        std::cout << std::setfill('0') << std::setw(2) << (data.length()-len)*100 / data.length() << "%\b\b\b";
        writeMemoryAddress(addr, getChunk(data, offs, PACKET_LEN));
        offs += PACKET_LEN;
        addr += PACKET_LEN;
        len -= PACKET_LEN;
    }
    QByteArray lastChunk = getChunk(data, offs, len);
    while (lastChunk.length() < PACKET_LEN)
    {
        lastChunk.append(0xFF);
    }
    writeMemoryAddress(addr, lastChunk);
    std::cout << "Done\n";
}

void test(QextSerialPort *serial)
{
    qDebug() << "Starting test...\t\t";
    QTime start = QTime::currentTime();
    QByteArray testData;
    for (int i = 0; i < 1024; i++)
    {
        uint8_t byte = i & 0xFF;
        testData.append(byte);
    }
    serial->write(testData);
    while (serial->bytesAvailable() < 1024);
    qDebug() << serial->bytesAvailable();

    qDebug() << "Test Complete" << 1024.0*8.0/(0.001*start.msecsTo(QTime::currentTime())) << "bps";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc != 3)
    {
        qDebug() << "Usage: AithonProgram <COM PORT> <BIN FILE>";
        return 1;
    }

    QFile file(argv[2]);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open binary file:" << argv[2];
        return 1;
    }
    std::cout << "Reading binary file...\t\t";
    QByteArray data = file.readAll();
    std::cout << "Done\n";

    std::cout << "Opening serial port...\t\t";
    port = new QextSerialPort(argv[1]);
    port->setBaudRate(BAUD115200);
    port->setTimeout(5000);
    if (!port->open(QextSerialPort::ReadWrite))
    {
        error("Could not open serial port.");
    }
    std::cout << "Done\n";

    std::cout << "Initalizing...\t\t\t";
    initChip();
    std::cout << "Done\n";

    eraseMemory();
    writeMemory(START_ADDR, data);
    startProgram();

    return 0;
}
