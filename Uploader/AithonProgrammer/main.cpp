#include <QtCore/QCoreApplication>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QFile>
#include <QTime>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <unistd.h>
#include <qmath.h>
#include <QObject>

#ifdef Q_OS_WIN32
#include <windows.h>
#define SLEEP(x) Sleep(uint(x))
#else
#define SLEEP(x) usleep(uint(1000*x))
#endif

#define USB_ST_VID          0x0483
#define USB_STM32F4_PID     0x5740
#define USB_BOOTLOADER_PID  0x5741
#define PACKET_LEN          1024
#define MAX_RETRIES         5
#define SYNC_RETRIES        100

// timeouts
#define DEFAULT_TIMEOUT     1000
#define SYNC_TIMEOUT        50
#define FLASH_TIMEOUT       5000

// control characters
#define SYNC                0xA5
#define BUSY                0xB2
#define ACK                 0x79
#define NACK                0x1F
#define ERASE_FLASH_START   0x43
#define ERASE_FLASH_STATUS  0x8C
#define SET_ADDR            0x31
#define FILL_BUFFER         0xC7
#define COMMIT_BUFFER       0x6E
#define START_PROGRAM       0x2A

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
    RECV_BUSY,
    FSM_RETRY
} error_t;

typedef enum {
    FSM_RESET,
    FSM_SYNC,
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
bool _debug = false;
qint64 startTime = 0;


void debug(QString msg)
{
    qint64 timeDiff = QDateTime::currentMSecsSinceEpoch() - startTime;
    if (_debug)
        std::cout << "\nDEBUG[" << timeDiff << "]: " << msg.toStdString() << "\n";
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
    std::cout.flush();
}

QString getCOMPort(bool isBootloader)
{
    int productId = isBootloader?USB_BOOTLOADER_PID:USB_STM32F4_PID;
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
    {
        if (info.vendorID > 0 && info.vendorID < 0xFFFF)
            debug(QString("ID = 0x%1:0x%2, Port = %3, PhysName = %4").arg(QString::number(info.vendorID, 16), QString::number(info.productID, 16), info.portName, info.physName));
        if (info.vendorID == USB_ST_VID && info.productID == productId)
        {
#ifdef Q_OS_LINUX
            return info.physName;
#else
            return info.portName;
#endif
        }
    }
    return QString("");
}

bool isAithonCDC()
{
    return _port->portName() == getCOMPort(false) || _port->portName() == getCOMPort(true);
}

bool isAithonCDCBootloader()
{
    return _port->portName() == getCOMPort(true);
}

bool isPortActive(QString port)
{
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
    {
        if (info.portName == port)
        {
            return true;
        }
    }
    return false;
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

    switch (data)
    {
    case ACK:
        _error = SUCCESS;
        break;
    case NACK:
        _error = RECV_NACK;
        break;
    case BUSY:
        _error = RECV_BUSY;
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
    else if (_error == RECV_BUSY)
        debug("Got busy response from "+state+".");
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
    _port->setBaudRate(BAUD9600);
    _port->setTimeout(1000);
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

state_t resetChip()
{
    if (isAithonCDCBootloader())
    {
        // We don't need to reset the board.
        // Nothing to do here.
    }
    if (isAithonCDC())
    {
        // send a 0x023 seqeunce using RTS/DTR to do a software reset of the board
        _port->setRts(false);
        _port->setDtr(false);
        SLEEP(100);
        _port->setRts(true);
        _port->setDtr(false);
        SLEEP(100);
        _port->setDtr(true);
        debug("Reset board.");

        // reopen the port
        _port->close();
        delete _port;
        debug("Deleted port.");

        QString comPort;
        while (comPort.length() == 0)
        {
            SLEEP(100);
            comPort = getCOMPort(true);
        }

        _port = new QextSerialPort(comPort);
        _port->setBaudRate(BAUD9600);
        _port->setTimeout(1000);
        if (!_port->open(QextSerialPort::ReadWrite))
            error("Could not open serial port.");
        debug("Opened port.");
    }
    else
    {
        // it's probably going to one of the UARTs so use SYNC bytes to reset
        debug("Skipping reset for UART.");
    }
    return FSM_SYNC;
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
    writeByte(ERASE_FLASH_START);
    waitForACK(FLASH_TIMEOUT);

    // check the status
    while (true)
    {
        writeByte(ERASE_FLASH_STATUS);
        waitForACK(FLASH_TIMEOUT);
        if (_error == RECV_BUSY)
        {
            debug("FLASH BUSY");
            SLEEP(10);
        }
        else
        {
            break;
        }
    }

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

void doProgramFSM()
{
    state_t state = FSM_RESET;
    state_t nextState = FSM_RESET;
    int packetNum = 0;
    int retries = MAX_RETRIES;
    
    while (true)
    {
        debug("Current State = "+QString::number(state));
        switch (state)
        {
        case FSM_RESET:
            std::cout << "Resetting Aithon...\t\t   ";
            nextState = resetChip();
            if (nextState != state)
                std::cout << "\b\b\bDone\n";
            break;
        case FSM_SYNC:
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
            if (state != FSM_RESET)
            {
                // Best effort attempt to resync.
                doSync(1);
            }
        }
        else
        {
            retries = MAX_RETRIES;
        }
    }
}

void displayUsage()
{
    std::cout << "Usage:\n";
    std::cout << "       AithonProgrammer.exe [-d] detect\n";
    std::cout << "       AithonProgrammer.exe [-d] program <BINARY_FILE_PATH> [COM_PORT]\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    startTime = QDateTime::currentMSecsSinceEpoch();

    if (argc < 2)
    {
        displayUsage();
        return -1;
    }

    QString cmd = argv[1];
    if (!cmd.compare("-d", Qt::CaseInsensitive))
    {
        _debug = true;
        cmd = argv[2];
        if (argc > 3)
        {
            argv[2] = argv[3];
            argc--;
        }

    }
    QString comPort = getCOMPort(true); // see if it's in the bootloader first
    if (comPort.length() == 0)
        comPort = getCOMPort(false);
    debug(comPort);
    if (!cmd.compare("detect", Qt::CaseInsensitive))
    {
        if (comPort.length() == 0)
        {
            std::cout << "No Aithon board detected.\n";
        }
        else
        {
            std::cout << "Aithon board detected on:\n";
            std::cout << comPort.toStdString() << "\n";
        }
    }
    else if (!cmd.compare("program", Qt::CaseInsensitive) ||
             !cmd.compare("program_debug", Qt::CaseInsensitive))
    {
        if (argc == 4)
        {
            // user specified com port
            comPort = QString(argv[3]);
        }
        else if (comPort.length() == 0)
        {
            std::cout << "No Aithon board detected.\n";
            return -1;
        }
        readFile(QString(argv[2]));
        openPort(comPort);
        doProgramFSM();
        _port->close();
    }
    else if (!cmd.compare("test", Qt::CaseInsensitive))
    {
        if (argc == 3)
        {
            // user specified com port
            comPort = QString(argv[2]);
        }
        else if (comPort.length() == 0)
        {
            std::cout << "No Aithon board detected.\n";
            return -1;
        }
        openPort(comPort);
        for (int i = 0; i <= 255; i++)
        {
            writeByte((uint8_t)i);
        }
        _port->close();
    }
    else
    {
        displayUsage();
        return -1;
    }

    return 0;
}
