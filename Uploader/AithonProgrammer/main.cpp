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
#include <qmath.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#define SLEEP(x) Sleep(uint(x))
#else
#define SLEEP(x) sleep(uint(x))
#endif

#define START_ADDR      0x08008000
#define PACKET_LEN      1024
#define MAX_RETRIES     5

// timeouts
#define DEFAULT_TIMEOUT 1000
#define SYNC_TIMEOUT    500
#define ERASE_TIMEOUT   15000

// control characters
#define SYNC            0xA5
#define ACK             0x79
#define NACK            0x1F
#define ABORT           0x99
#define ERASE_FLASH     0x43
#define SET_ADDR        0x31
#define FILL_BUFFER     0xC7
#define COMMIT_BUFFER   0x6E
#define START_PROGRAM   0x2A

typedef enum {
    SUCCESS = 0,
    TIMEOUT,
    BAD_RESPONSE,
    RECV_NACK
} error_t;

QextSerialPort *_port = NULL;
QByteArray _programData;
error_t _error;


void error(QString msg)
{
    qDebug() << "ERROR: " << msg;
    if (_port != NULL && _port->isOpen())
        _port->close();
    exit(1);
}

void printStatus(int current, int total)
{
    std::cout << std::setfill('0') << std::setw(2) << (total-current)*100 / total << "%\b\b\b";
}


void writeByte(uint8_t byte)
{
    _port->write((const char *)&byte, 1);
}

uint8_t getByte(int timeout=DEFAULT_TIMEOUT)
{
    while (!_port->bytesAvailable())
    {
        if (timeout <= 0)
        {
            _error = TIMEOUT;
            return 0;
        }

        SLEEP(1);
        timeout--;
    }
    _error = SUCCESS;
    return (uint8_t) _port->read(1).at(0);
}

void waitForACK(int timeout=DEFAULT_TIMEOUT)
{
    uint8_t data = getByte(timeout);

    if (_error)
        return;

    switch (data)
    {
    case ACK:
        _error = SUCCESS;
        break;
    case NACK:
        _error = RECV_NACK;
        break;
    default:
        qDebug() << "Expected ACK or NACK - got "+QString("0x%1").arg((int)data, 0, 16);
        _error = BAD_RESPONSE;
        break;
    }
}

void processACKError(QString state)
{
    if (_error == TIMEOUT)
        error("Timed out waiting for ACK from "+state+".");
    else if (_error == RECV_NACK)
        error("Got unexpected NACK from "+state+".");
    else if (_error == BAD_RESPONSE)
        error("Got bad response from "+state+".");
}

void setAddress(uint32_t addr)
{
    writeByte(SET_ADDR);

    uint8_t checksum = 0;
    for (int i = 0; i < 4; i++)
        checksum ^= (addr >> (8 * i)) & 0xFF;

    _port->write((char *)&addr, 4);

    // wait for ACK
    waitForACK();
    processACKError("SET_ADDR");

    // compare checksum
    uint8_t checksum2 = getByte();
    if (_error == TIMEOUT)
        error("Did not receive checksum back from SET_ADDR.");
    else if (checksum2 != checksum)
        error("Got invalid checksum back from SET_ADDR.");
}

void writePacket(int packetNum)
{
    QByteArray data = _programData.mid(packetNum*PACKET_LEN, PACKET_LEN);

    // set address
    uint32_t addr = packetNum * PACKET_LEN + START_ADDR;
    setAddress(addr);

    // fill buffer
    writeByte(FILL_BUFFER);
    _port->write(data);

    // wait for ACK
    waitForACK();
    processACKError("FILL_BUFFER");

    // compare checksum
    uint8_t checksum = 0;
    for (int i = 0; i < PACKET_LEN; i++)
    {
        checksum ^= (uint8_t) data.at(i);
    }
    uint8_t checksum2 = getByte();
    if (_error == TIMEOUT)
        error("Did not receive checksum back from FILL_BUFFER.");
    else if (checksum2 != checksum)
        error("Got invalid checksum back from FILL_BUFFER.");

    // commit buffer to FLASH
    writeByte(COMMIT_BUFFER);

    // wait for ACK
    waitForACK(ERASE_TIMEOUT);
    processACKError("COMMIT_BUFFER");
}



void readFile(QString fileName)
{
    QFile file(fileName);
    if (fileName.right(4).compare(".bin"))
        error("Incorrect file type. Expected .bin file.");
    else if (!file.open(QIODevice::ReadOnly))
        error("Could not open binary file.");

    std::cout << "Reading binary file...\t\t";
    _programData = file.readAll();
    while (_programData.length() % PACKET_LEN)
        _programData.append(0xFF);
    std::cout << "Done\n";
}

void openPort(QString port)
{
    std::cout << "Opening serial port...\t\t";
    _port = new QextSerialPort(port);
    _port->setBaudRate(BAUD115200);
    _port->setTimeout(5000);
    if (!_port->open(QextSerialPort::ReadWrite))
        error("Could not open serial port.");
    std::cout << "Done\n";
}

void initChip()
{
    for (int i = 0; i < 50; i++)
    {
        writeByte(SYNC);
        waitForACK(SYNC_TIMEOUT);
        if (_error == SUCCESS)
            break;
        else if (_error == RECV_NACK)
            // Aithon may respond with a NACK if it can't currently be programmed.
            // This is not a current use-case, but may be useful in the future.
            error("Aithon board is not ready to be programmed.");
    }
    qDebug() << "Initialized chip!";
}

void eraseFlash()
{
    // send command
    writeByte(ERASE_FLASH);

    // wait for ACK of command
    waitForACK();
    processACKError("ERASE_FLASH");
    qDebug() << "Starting erasing FLASH!";

    // wait for ACK signaling that we're done erasing the flash
    waitForACK(ERASE_TIMEOUT);
    processACKError("ERASE_FLASH (done)");
    qDebug() << "Erased FLASH!";
}

void writeProgram()
{
    for (int i = 0; i < (_programData.length() / PACKET_LEN); i++)
    {
        std::cout << "Writing packet " << i << "...";
        writePacket(i);
        std::cout << "done\n";
        SLEEP(100);
    }
}

void startProgram()
{
    writeByte(START_PROGRAM);

    // wait for ACK
    waitForACK();
    processACKError("START_PROGRAM");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc != 3)
    {
        qDebug() << "Usage: AithonProgram <COM PORT> <BIN FILE>";
        return 1;
    }

    readFile(QString(argv[2]));
    openPort(QString(argv[1]));
    initChip();
    eraseFlash();
    writeProgram();
    startProgram();

    _port->close();
    return 0;
}

/* old code...

void initChip()
{
    writeByte(0x11);
    SLEEP(500);
    writeByte(0x11);
    SLEEP(100);
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
        printStatus(len, data.length());
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
    if (QString(argv[2]).right(4).compare(".bin"))
    {
        qDebug() << "Incorrect file type. Expected .bin file.";
        return 1;
    }
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
}*/
