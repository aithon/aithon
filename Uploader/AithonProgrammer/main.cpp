#include <QtCore/QCoreApplication>
#include "qextserialport.h"
#include "qextserialenumerator.h"
extern "C" {
#include "../avrdude/serial.h"
#include "../avrdude/avrdude.h"
}
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
#define RESEND_RETRIES      3
#define MAX_RETRIES         5
#define SYNC_RETRIES        100

// timeouts
#define DEFAULT_TIMEOUT     1000
#define SYNC_TIMEOUT        50
#define FLASH_TIMEOUT       5000


// Commands
#define SYNC               0x01
#define ERASE_FLASH_START  0x1B
#define ERASE_FLASH_STATUS 0x09
#define SET_ADDR           0x1A
#define CHECK_ADDR         0x3B
#define FILL_BUFFER        0x29
#define CHECK_BUFFER       0x06
#define COMMIT_BUFFER      0x28
#define START_PROGRAM      0x11

// Responses
#define ACK                0x40
#define NACK               0x80
#define BUSY               0xC0

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
    RECV_ZERO,
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
QString _portName;
QByteArray _programData;
error_t _error;
union filedescriptor portFD;
int _numPackets;
bool _debug = false;
qint64 startTime = 0;

void closePort(void);


void debug(QString msg)
{
    qint64 timeDiff = QDateTime::currentMSecsSinceEpoch() - startTime;
    if (_debug)
        std::cout << "\nDEBUG[" << timeDiff << "]: " << msg.toStdString() << "\n";
}

void error(QString msg)
{
    std::cout << "\nERROR: " << msg.toStdString() << "\n";
    closePort();
    exit(1);
}

void printStatus(int current, int total)
{
    std::cout << "\b\b\b" << std::setfill('0') << std::setw(2) << current*100 / total << "%";
    std::cout.flush();
}


void openPort(QString port)
{
    QByteArray ba = port.toLocal8Bit();
    ser_open(ba.data(), 9600, &portFD);
    _portName = port;
    //ser_open(name, 9600, &portFD);

    //_portName = port;
    //_port = new QextSerialPort(port);
    //_port->setBaudRate(BAUD9600);
    //_port->setTimeout(1000);
    //if (!_port->open(QextSerialPort::ReadWrite))
    //    error("Could not open serial port.");
}

void flushPort(void)
{
    ser_drain(&portFD, 0);

    // empty output buffer
    //_port->flush();
    // 1ms sleep to reduce chance of race conditions
    //SLEEP(1);
    // empty input buffer
    //_port->readAll();
}

void sendReset(void)
{
    ser_set_dtr_rts(&portFD, 0);
    SLEEP(100);
    flushPort();
    ser_set_dtr_rts(&portFD, 2);
    SLEEP(100);
    flushPort();
    ser_set_dtr_rts(&portFD, 3);
    SLEEP(100);
    flushPort();

    // send a 0x023 sequence using RTS/DTR to do a software reset of the board
    //_port->setRts(false);
    //_port->setDtr(false);
    //SLEEP(100);
    //_port->setRts(true);
    //_port->setDtr(false);
    //SLEEP(100);
    //_port->setDtr(true);
}

void closePort(void)
{
    ser_close(&portFD);
    //if (!_port)
    //    return;
    //_port->close();
    //delete _port;
}


void writeByte(uint8_t byte)
{
    ser_send(&portFD, &byte, 1);
    //_port->write((const char *)&byte, 1);
}

uint8_t getByte(int timeout)
{
  int ret;
  unsigned char buf;
  ret = ser_recv(&portFD, &buf, 1, timeout);

  if (ret == -1) {
      _error = TIMEOUT;
      return 0;
  } else {
      _error = SUCCESS;
      return buf;
  }
  
//    while (!_port->bytesAvailable())
//    {
//        if (timeout <= 0)
//        {
//            _error = TIMEOUT;
//            return 0;
//        }
//
//        SLEEP(1);
//        timeout--;
//    }
//    _error = SUCCESS;
//    return (uint8_t) _port->read(1).at(0);
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
    return _portName == getCOMPort(false) || _portName == getCOMPort(true);
}

bool isAithonCDCBootloader()
{
    return _portName == getCOMPort(true);
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


void waitForACK(uint8_t commandSent, int timeout=DEFAULT_TIMEOUT)
{
    uint8_t data = 0;
    int tries = 0;
    while (timeout > 0 && !data)
    {
        data = getByte(timeout);
        tries++;
    }
    if (!data)
    {
        _error = RECV_ZERO;
        return;
    }
    uint8_t response = data & 0xC0;
    uint8_t command = data & 0x3F;

    if (command != commandSent)
    {
        _error = BAD_RESPONSE;
        debug(QString("Got response to incorrect command (Sent 0x%1). Received byte: 0x%2 ").arg(QString::number((int)commandSent, 16), QString::number((int)data, 16)));
        return;
    }

    switch (response)
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
        debug("Got invalid response bits!");
        _error = BAD_RESPONSE;
        break;
    }
}

void writeAndAck(uint8_t byte, int timeout=DEFAULT_TIMEOUT)
{
    for (int i = 0; i < RESEND_RETRIES; i++)
    {
        std::cout << "\twriting byte: " << byte; 
        writeByte(byte);
        waitForACK(byte, timeout);
        if (_error != RECV_ZERO)
            break;
    }
}

void write2AndAck(uint8_t byte, uint8_t byte2, int timeout=DEFAULT_TIMEOUT)
{
    for (int i = 0; i < RESEND_RETRIES; i++)
    {
        writeByte(byte);
        writeByte(byte2);
        waitForACK(byte, timeout);
        if (_error != RECV_ZERO)
            break;
    }
}

void writeBytesAndAck(uint8_t byte, uint8_t *bytes, int numBytes, int timeout=DEFAULT_TIMEOUT)
{
    for (int i = 0; i < RESEND_RETRIES; i++)
    {
        writeByte(byte);
        for (int j = 0; j < numBytes; j++)
            writeByte(bytes[j]);
        waitForACK(byte, timeout);
        if (_error != RECV_ZERO)
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
    else if (_error == RECV_ZERO)
        debug("Got 0 byte from "+state+".");
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

bool doSync(int attempts = SYNC_RETRIES)
{
    for (int i = 0; i < attempts; i++)
    {
        std::cout<< "trying to sync...\n";
        // small delay before trying
        SLEEP(SYNC_TIMEOUT);
        flushPort();

        // send SYNC command and expect SYNC response
        writeAndAck(SYNC, SYNC_TIMEOUT);
        debugPrintError("SYNC");
        if (!_error)
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
    else if (isAithonCDC())
    {
        sendReset();
        debug("Reset board.");

        // reopen the port
        closePort();
        debug("Deleted port.");
        SLEEP(1000);

        QString comPort;
        while (comPort.length() == 0)
        {
            SLEEP(100);
            comPort = getCOMPort(true);
        }


        openPort(comPort);
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
    writeAndAck(ERASE_FLASH_START, FLASH_TIMEOUT);

    // check the status
    while (true)
    {
        writeAndAck(ERASE_FLASH_STATUS, FLASH_TIMEOUT);
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
    // send address
    // wait for ACK
    uint32_t addr = packetNum * PACKET_LEN;
    writeBytesAndAck(SET_ADDR, (uint8_t *)&addr, 4);
    debugPrintError("SET_ADDR");
    CHECK_FOR_ERROR(FSM_SET_ADDR);

    // compute checksum
    uint8_t checksum = 0;
    for (int i = 0; i < 4; i++)
        checksum ^= (addr >> (8 * i)) & 0xFF;

    // send checksum
    // wait for ACK
    write2AndAck(CHECK_ADDR, checksum);
    debugPrintError("CHECK_ADDR");
    CHECK_FOR_ERROR(FSM_SET_ADDR);

    return FSM_FILL_BUFFER;
}

state_t fillBuffer(const int packetNum)
{
    QByteArray data = _programData.mid(packetNum*PACKET_LEN, PACKET_LEN);
    if (data.length() != PACKET_LEN)
        error("Invalid packet length!");

    // send data packet
    // wait for ACK
    writeBytesAndAck(FILL_BUFFER, (uint8_t *)data.data(), PACKET_LEN);
    debugPrintError("FILL_BUFFER");
    CHECK_FOR_ERROR(FSM_FILL_BUFFER);

    // calculate checksum
    uint8_t checksum = 0;
    for (int i = 0; i < PACKET_LEN; i++)
        checksum ^= (uint8_t) data.at(i);

    // send checksum
    // wait for ACK
    write2AndAck(CHECK_BUFFER, checksum);
    debugPrintError("CHECK_BUFFER");
    CHECK_FOR_ERROR(FSM_FILL_BUFFER);

    return FSM_COMMIT_BUFFER;
}

state_t commitBuffer(int &packetNum)
{
    // commit buffer to FLASH
    // wait for ACK
    writeAndAck(COMMIT_BUFFER, FLASH_TIMEOUT);
    debugPrintError("COMMIT_BUFFER");
    CHECK_FOR_ERROR(FSM_COMMIT_BUFFER);


    if (++packetNum == _numPackets)
        return FSM_START_PROGRAM;
    else
        return FSM_SET_ADDR;
}

state_t startProgram()
{
    // wait for ACK
    writeAndAck(START_PROGRAM);
    closePort();
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
            writeByte(SYNC);
            SLEEP(100);
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
        std::cout << "Opening serial port...\t\t";
        openPort(comPort);
        std::cout << "Done\n";
        doProgramFSM();
        closePort();
    }
    else if (!cmd.compare("test", Qt::CaseInsensitive))
    {
        if (comPort.length() == 0)
        {
            std::cout << "No Aithon board detected.\n";
            return -1;
        }
        openPort(comPort);
        for (int i = 0; i <= 255; i++)
        {
            writeByte((uint8_t)i);
            debug("WRITE: "+QString::number(i));
            while (_port->bytesAvailable())
                debug("READ: "+QString::number((uint8_t)_port->read(1).at(0)));
        }
        closePort();
    }
    else
    {
        displayUsage();
        return -1;
    }

    return 0;
}
