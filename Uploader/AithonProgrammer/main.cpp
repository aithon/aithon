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

// uncomment to enable debug printing
//#define DEBUG

#define PACKET_LEN      4096
#define MAX_RETRIES     5
#define SYNC_RETRIES    100

// timeouts
#define DEFAULT_TIMEOUT 1000
#define SYNC_TIMEOUT    500
#define FLASH_TIMEOUT   15000

// control characters
#define SYNC            0xA5
#define ACK             0x79
#define NACK            0x1F
#define ERASE_FLASH     0x43
#define SET_ADDR        0x31
#define FILL_BUFFER     0xC7
#define COMMIT_BUFFER   0x6E
#define START_PROGRAM   0x2A

#define CHECK_FOR_ERROR(state) \
    do { \
        if (_error) { \
            _error = FSM_RETRY; \
            return state; \
        } \
    } while(0)


typedef enum {
    SUCCESS = 0,
    TIMEOUT,
    BAD_RESPONSE,
    RECV_NACK,
    FSM_RETRY
} error_t;

typedef enum {
    FSM_INIT,
    FSM_ERASE_FLASH,
    FSM_SET_ADDR,
    FSM_FILL_BUFFER,
    FSM_COMMIT_BUFFER,
    FSM_START_PROGRAM,
    FSM_QUIT
} state_t;

QextSerialPort *_port = NULL;
QByteArray _programData;
error_t _error;
int _numPackets;


void debug(QString msg)
{
#ifdef DEBUG
    std::cout << "\nDEBUG: " << msg.toStdString() << "\n";
#else
    (void)msg;
#endif
}

void error(QString msg)
{
    std::cout << "\nERROR: " << msg.toStdString() << "\n";
    if (_port != NULL && _port->isOpen())
        _port->close();
    exit(1);
}

void printStatus(int current, int total)
{
    std::cout << "\b\b\b" << std::setfill('0') << std::setw(2) << current*100 / total << "%";
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
        debug("Expected ACK or NACK - got "+QString("0x%1").arg((int)data, 0, 16));
        _error = BAD_RESPONSE;
        break;
    }
}

void debugPrintError(QString state)
{
    if (_error == TIMEOUT)
        debug("Timed out waiting for ACK from "+state+".");
    else if (_error == RECV_NACK)
        debug("Got unexpected NACK from "+state+".");
    else if (_error == BAD_RESPONSE)
        debug("Got bad response from "+state+".");
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
    _numPackets = _programData.length() / PACKET_LEN;
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

bool doSync(int attempts = SYNC_RETRIES)
{
    for (int i = 0; i < attempts; i++)
    {
        // small delay before trying
        SLEEP(SYNC_TIMEOUT);
        // empty output buffer
        _port->flush();
        // 1ms sleep to reduce chance of race conditions
        SLEEP(1);
        // empty input buffer
        _port->readAll();

        // send SYNC command and expect SYNC response
        writeByte(SYNC);
        uint8_t response = getByte(SYNC_TIMEOUT);
        if (!_error && response == SYNC)
        {
            debug("Synced with Aithon.");
            return true;
        }
    }
    debug("Sync failed.\n");
    return false;
}

state_t initChip()
{
    if (doSync())
        return FSM_ERASE_FLASH;
    else
        return FSM_QUIT;
}

state_t eraseFlash()
{
    // send command
    writeByte(ERASE_FLASH);

    // wait for ACK of command
    waitForACK();
    debugPrintError("ERASE_FLASH");
    CHECK_FOR_ERROR(FSM_ERASE_FLASH);

    // wait for ACK signaling that we're done erasing the flash
    waitForACK(FLASH_TIMEOUT);
    debugPrintError("ERASE_FLASH (2)");
    CHECK_FOR_ERROR(FSM_ERASE_FLASH);

    return FSM_SET_ADDR;
}

state_t setAddress(const int packetNum)
{
    // set address
    uint32_t addr = packetNum * PACKET_LEN;
    writeByte(SET_ADDR);

    // compute checksum
    uint8_t checksum = 0;
    for (int i = 0; i < 4; i++)
        checksum ^= (addr >> (8 * i)) & 0xFF;

    // send address
    _port->write((char *)&addr, 4);

    // wait for ACK
    waitForACK();
    debugPrintError("SET_ADDR");
    CHECK_FOR_ERROR(FSM_SET_ADDR);

    // compare checksum for address
    uint8_t checksum2 = getByte();
    debugPrintError("SET_ADDR checksum");
    CHECK_FOR_ERROR(FSM_SET_ADDR);

    if (checksum2 != checksum)
    {
        debug("Got invalid checksum back from SET_ADDR.");
        _error = FSM_RETRY;
        return FSM_SET_ADDR;
    }

    return FSM_FILL_BUFFER;
}

state_t fillBuffer(const int packetNum)
{
    QByteArray data = _programData.mid(packetNum*PACKET_LEN, PACKET_LEN);
    if (data.length() != PACKET_LEN)
        error("Invalid packet length!");

    // send data packet
    writeByte(FILL_BUFFER);
    _port->write(data);

    // wait for ACK
    waitForACK();
    debugPrintError("FILL_BUFFER");
    CHECK_FOR_ERROR(FSM_FILL_BUFFER);

    // calculate checksum
    uint8_t checksum = 0;
    for (int i = 0; i < PACKET_LEN; i++)
        checksum ^= (uint8_t) data.at(i);

    // get checksum
    uint8_t checksum2 = getByte();
    debugPrintError("FILL_BUFFER (checksum)");
    CHECK_FOR_ERROR(FSM_FILL_BUFFER);

    // compare checksum
    if (checksum2 != checksum)
    {
        debug("Got invalid checksum back from FILL_BUFFER.");
        _error = FSM_RETRY;
        return FSM_FILL_BUFFER;
    }

    return FSM_COMMIT_BUFFER;
}

state_t commitBuffer(int &packetNum)
{
    // commit buffer to FLASH
    writeByte(COMMIT_BUFFER);

    // wait for ACK
    waitForACK(FLASH_TIMEOUT);
    debugPrintError("COMMIT_BUFFER");
    CHECK_FOR_ERROR(FSM_COMMIT_BUFFER);


    if (++packetNum == _numPackets)
        return FSM_START_PROGRAM;
    else
        return FSM_SET_ADDR;
}

state_t startProgram()
{
    writeByte(START_PROGRAM);
    // wait for ACK
    waitForACK();
    debugPrintError("START_PROGRAM");
    if (_error)
    {
        _error = FSM_RETRY;
        return FSM_START_PROGRAM;
    }

    return FSM_QUIT;
}

void doFSM()
{
    state_t state = FSM_INIT;
    state_t nextState = FSM_INIT;
    int packetNum = 0;
    int retries = MAX_RETRIES;

    while (true)
    {
        switch (state)
        {
        case FSM_INIT:
            std::cout << "\rSyncing with Aithon...\t\t   ";
            nextState = initChip();
            if (nextState != state)
                std::cout << "\b\b\bDone\n";
            break;
        case FSM_ERASE_FLASH:
            std::cout << "\rErasing FLASH...\t\t   ";
            nextState = eraseFlash();
            if (nextState != state)
                std::cout << "\b\b\bDone\n";
            break;
        case FSM_SET_ADDR:
            std::cout << "\rWriting program data...\t\t   ";
            printStatus(packetNum, _numPackets);
            nextState = setAddress(packetNum);
            break;
        case FSM_FILL_BUFFER:
            std::cout << "\rWriting program data...\t\t   ";
            printStatus(packetNum, _numPackets);
            nextState = fillBuffer(packetNum);
            break;
        case FSM_COMMIT_BUFFER:
            std::cout << "\rWriting program data...\t\t   ";
            printStatus(packetNum, _numPackets);
            nextState = commitBuffer(packetNum);
            if (nextState == FSM_START_PROGRAM)
                std::cout << "\b\b\bDone\n";
            break;
        case FSM_START_PROGRAM:
            std::cout << "\rStarting program...\t\t   ";
            nextState = startProgram();
            if (nextState != state)
                std::cout << "\b\b\bDone\n";
            break;
        case FSM_QUIT:
            return;
        default:
            error("Illegal state!");
        }
        state = nextState;

        if (_error == FSM_RETRY)
        {
            if (retries-- == 0)
            {
                error("No more retries!");
            }
            // Best effort attempt to resync.
            doSync(1);
        }
        else
        {
            retries = MAX_RETRIES;
        }
    }
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

    doFSM();

    _port->close();
    return 0;
}
