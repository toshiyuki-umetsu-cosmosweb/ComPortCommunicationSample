#include <tchar.h>
#include <Windows.h>

#include <cstdio>
#include <system_error>

#if WINVER >= _WIN32_WINNT_WIN10
/**
 * Note: GetCommPorts()を使用するにはOneCore.libが必要。
 */
#pragma comment(lib, "OneCore.lib")
#endif

#include "WindowsErrorCategory.h"
#include "SerialPort.h"

#if WINVER < _WIN32_WINNT_WIN10
/**
 * 該当ポートが存在するかどうかを取得する。
 *
 * @param port_name ポート名
 * @retval true ポートが存在する。
 * @retval false ポートが存在しない。
 * @note
 * これは該当ポートがオープンできるかどうかで判定しているので、排他アクセスでオープンされている場合には失敗する。
 * そのため、ポートがあるのにfalseが返るという現象が発生する。
 */
static bool is_port_exists(const char* port_name) {
    char com_device_path[256];

    sprintf_s(com_device_path, sizeof(com_device_path) - 1, "\\\\.\\%s", port_name);

    HANDLE handle = CreateFileA(com_device_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        return true;
    }
    else {
        return false;
    }

}
#endif

const uint32_t SerialPort::StopBitsOne = ONESTOPBIT;
const uint32_t SerialPort::StopBitsOne5 = ONE5STOPBITS;
const uint32_t SerialPort::StopBitsTwo = TWOSTOPBITS;
const uint32_t SerialPort::ParityNone = PARITY_NONE;
const uint32_t SerialPort::ParityEven = PARITY_EVEN;
const uint32_t SerialPort::ParityOdd = PARITY_ODD;
const uint32_t SerialPort::CtsFlowDisable = FALSE;
const uint32_t SerialPort::CtsFlowEnable = TRUE;
const uint32_t SerialPort::RtsControlDisable = RTS_CONTROL_DISABLE;
const uint32_t SerialPort::RtsControlEnable = RTS_CONTROL_ENABLE;
const uint32_t SerialPort::RtsControlHandShake = RTS_CONTROL_HANDSHAKE;
const uint32_t SerialPort::RtsControlToggle = RTS_CONTROL_TOGGLE;
const uint32_t SerialPort::ErrorBreak = CE_BREAK;
const uint32_t SerialPort::ErrorFrame = CE_FRAME;
const uint32_t SerialPort::ErrorOverrRun = CE_OVERRUN;
const uint32_t SerialPort::ErrorReceiveOverflow = CE_RXOVER;
const uint32_t SerialPort::ErrorReceiveParity = CE_RXPARITY;

bool SerialPort::enumerate_ports(std::vector<std::string>* plist) {
    if (plist == nullptr) {
        return false;
    }

    (*plist).clear();

#if WINVER >= _WIN32_WINNT_WIN10
    ULONG port_numbers[256];
    ULONG found_port_count;
    if (GetCommPorts(port_numbers, sizeof(port_numbers) / sizeof(ULONG), &found_port_count) != ERROR_SUCCESS) {
        return false;
    }
    for (ULONG i = 0; i < found_port_count; i++) {
        char port_name[32];
        sprintf_s(port_name, sizeof(port_name) - 1, "COM%d", port_numbers[i]);
        (*plist).push_back(std::string(port_name));
    }
    return true;
#else
    for (int port_no = 0; port_no < 255; port_no++) {
        char port_name[32];
        sprintf_s(port_name, sizeof(port_name) - 1, "COM%d", port_no);
        if (is_port_exists(port_name)) {
            (*plist).push_back(std::string(port_name));
        }
    }
    return true;
#endif
}

SerialPort::SerialPort(const std::string& port_name)
    : m_port_handle(INVALID_HANDLE_VALUE), m_port_name(port_name),
    m_baudrate(9600), m_databits(8), m_parity(ParityNone),
    m_stopbits(StopBitsOne), m_cts_flow(CtsFlowDisable), m_rts_control(RtsControlDisable) {

}

SerialPort::SerialPort(const std::string& port_name, const SerialPort& ref_port) 
    : m_port_handle(INVALID_HANDLE_VALUE), m_port_name(port_name),
    m_baudrate(ref_port.m_baudrate), m_databits(ref_port.m_databits), m_parity(ref_port.m_parity),
    m_stopbits(ref_port.m_stopbits), m_cts_flow(ref_port.m_cts_flow), m_rts_control(ref_port.m_rts_control) {

}

SerialPort::~SerialPort(void) {
    if (is_opened()) {
        close();
    }
}

void SerialPort::open(void) {
    if (is_opened()) {
        close();
    }
    char com_device_path[256];

    sprintf_s(com_device_path, sizeof(com_device_path) - 1, "\\\\.\\%s", m_port_name.c_str());
    m_port_handle = CreateFileA(com_device_path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (m_port_handle == INVALID_HANDLE_VALUE) {
        DWORD ev = GetLastError();
        throw std::system_error(ev, windows_error_category());
    }

    apply_settings();
}

void SerialPort::close(void) {
    if (is_opened()) {
        CloseHandle(m_port_handle);
        m_port_handle = INVALID_HANDLE_VALUE;
    }
}

void SerialPort::apply_settings(void) {
    if (!is_opened()) { // オープンしてない？
        return;
    }

    DCB dcb;
    if (GetCommState(m_port_handle, &dcb) == FALSE) {
        return;
    }

    dcb.BaudRate = m_baudrate;
    dcb.ByteSize = m_databits;
    dcb.Parity = m_parity;
    dcb.StopBits = m_stopbits;
    dcb.fOutxCtsFlow = m_cts_flow;
    dcb.fRtsControl = m_rts_control;

    SetCommState(m_port_handle, &dcb);
}

int SerialPort::send(const uint8_t* data, uint32_t length, int timeout_millis) {
    if (data == nullptr) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }
    if (length == 0) {
        return 0;
    }

    OVERLAPPED write_req;
    ZeroMemory(&write_req, sizeof(write_req));
    write_req.hEvent = NULL;
    DWORD transferred = 0;
    if (WriteFile(m_port_handle, data, length, &transferred, &write_req)) {
        return static_cast<int>(transferred);
    }
    else {
        auto err = GetLastError();
        if (err != ERROR_IO_PENDING) {
            SetLastError(err);
            return -1;
        }
        else {
            return wait_io(&write_req, timeout_millis);
        }
    }
}

int SerialPort::receive(uint8_t* buf, uint32_t bufsize, int timeout_millis) {
    if (buf == nullptr) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }
    if (bufsize == 0) {
        return 0;
    }

    DWORD errors;
    COMSTAT com_stat;
    if (!ClearCommError(m_port_handle, &errors, &com_stat)) {
        // Error number was set by COM.
        return -1;
    }
    else {
        // エラーハンドリング
        if (errors != 0) {
            handle_errors(errors);
        }
    }

    int io_length = (timeout_millis == 0) ? min(com_stat.cbInQue, bufsize) : bufsize;

    OVERLAPPED read_req;
    ZeroMemory(&read_req, sizeof(read_req));
    read_req.hEvent = NULL;
    DWORD transferred = 0;
    if (ReadFile(m_port_handle, buf, io_length, &transferred, &read_req)) {
        return static_cast<int>(transferred);
    }
    else {
        auto err = GetLastError();
        if (err != ERROR_IO_PENDING) {
            SetLastError(err);
            return -1;
        }
        else {
            return wait_io(&read_req, timeout_millis);
        }
    }
}

int SerialPort::wait_io(LPOVERLAPPED req, int timeout_millis) {
    auto begin = GetTickCount64();
    while (is_opened()) {
        if (HasOverlappedIoCompleted(req)) {
            break;
        }
        else if ((timeout_millis >= 0) // タイムアウト時間が有効？
            && ((GetTickCount64() - begin) >= timeout_millis)) { // タイムアウト時間が経過した？
            if (!CancelIoEx(m_port_handle, req)) {
                // Error number was set by CancelIoEx().
                return -1;
            }
            break;
        }
        SwitchToThread();
    }

    DWORD transferred = 0;
    if (!GetOverlappedResult(m_port_handle, req, &transferred, TRUE)) {
        // Error number was set by GetOverlappedResult().
        return -1;
    }
    else {
        return static_cast<int>(transferred);
    }
}

