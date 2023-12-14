#include <memory>
#include <thread>
#include <sstream>
#include "StandardIo.h"

StandardIo& StandardIo::instance(void) {
#if 0
    static std::unique_ptr<StandardIo> io;
    if (!io) {
        io.reset(new StandardIo());
        (*io).init();
    }
    return (*io);
#else
    static StandardIo stdio;
    if (!stdio.is_initialized()) {
        stdio.init();
    }
    return stdio;
#endif
}

bool StandardIo::Terminated = false;

const DWORD StandardIo::LineInputModeFunctions =
    ENABLE_ECHO_INPUT // ECHO�o�b�N�@�\(���͕����̃G�R�[�o�b�N)
    | ENABLE_INSERT_MODE // �C���T�[�g�@�\(�o�b�N�X�y�[�X�L�[��J�[�\���ړ��Ɠ��͏���)
    | ENABLE_LINE_INPUT // ���C���P�ʂł̓��͋@�\(���s�܂œ�����ReadFile�œǂݏo����)
    | ENABLE_QUICK_EDIT_MODE;// �N�C�b�N�G�f�B�b�g�@�\(�}�E�X�ɂ��̈�I���ƁA�E�N���b�N�ŃR�s�[)

StandardIo::StandardIo(void)
    : m_initialized(false), m_max_read_length(256), m_prev_input_data('\0') {
}
StandardIo::~StandardIo(void) {
}

bool StandardIo::init(void) {
    init_std_handle(&m_input, STD_INPUT_HANDLE);
    init_std_handle(&m_output, STD_OUTPUT_HANDLE);
    init_std_handle(&m_error, STD_ERROR_HANDLE);

    if (m_input.is_console) {
        modify_console_mode(&m_input, LineInputModeFunctions, false);
    }

    SetConsoleCtrlHandler(console_handler_proc, TRUE);

    if (is_input_valid()) {
        // ��M�X���b�h�J�n
        std::thread thread([this]() { this->receiver_thread_proc(); });
        thread.detach();
    }
    else {
        Terminated = true;
    }

    m_initialized = true;

    return true;
}

void StandardIo::init_std_handle(StandardIo::std_io* pstdio, DWORD handle_type) {
    auto handle = GetStdHandle(handle_type);
    if ((handle == INVALID_HANDLE_VALUE) || (handle == nullptr)) {
        // �Θb�^�R���\�[���Ɛڑ�����Ă��Ȃ����A���_�C���N�g����Ă��Ȃ��ꍇ�B
        (*pstdio).handle = INVALID_HANDLE_VALUE; // nullptr���Ԃ�ꍇ�ɂ�INVALID_HANDLE_VALUE�ɒu������
        (*pstdio).is_console = false;
        (*pstdio).mode = 0;
    }
    else {
        DWORD tmp;
        if (GetConsoleMode(handle, &tmp)) {
            (*pstdio).handle = handle;
            (*pstdio).is_console = true;
            (*pstdio).mode = tmp;
        }
        else {
            (*pstdio).handle = handle;
            (*pstdio).is_console = false;
            (*pstdio).mode = 0;
        }
    }
    
    return ;
}

bool StandardIo::modify_console_mode(std_io* pstdio, DWORD functions, bool is_enabled) {
    if (!(*pstdio).is_console) {
        return false;
    }

    DWORD new_mode;
    if (is_enabled) {
        new_mode = (*pstdio).mode | functions;
    }
    else {
        new_mode = (*pstdio).mode & ~functions;
    }
    if (new_mode == (*pstdio).mode) { // ���[�h�ɕύX�Ȃ��H
        return true;
    }

    if (SetConsoleMode((*pstdio).handle, new_mode)) {
        (*pstdio).mode = new_mode;
        return true;
    }
    else
    {
        return false;
    }
}

std::string StandardIo::read_line(void) {
    std::ostringstream oss;

    while (!is_input_EOF()) { // �I�[���m���Ă��Ȃ��H
        uint8_t c;
        if (!m_input_data.empty()) {
            {
                std::lock_guard<std::mutex> lock(m_input_lock);
                c = m_input_data.front();
                m_input_data.pop();
            }
            oss << c;
            if (c == '\n') {
                break;
            }
        }
        else {
            std::this_thread::yield();
        }
    }

    return oss.str();
}

bool StandardIo::read_line(char* buf, size_t bufsize, size_t* plength) {
    if ((buf == nullptr) || (bufsize < 1)) {
        return false;
    }

    size_t read_length = 0;
    while (!is_input_EOF() // �I�[���m���Ă��Ȃ��H
        && (read_length < (bufsize - 1))) { // �ǂݏo���������� bufsize - 1 �����H
        uint8_t c;
        if (!m_input_data.empty()) {
            {
                std::lock_guard<std::mutex> lock(m_input_lock);
                c = m_input_data.front();
                m_input_data.pop();
            }
            buf[read_length] = c;
            read_length++;
            if (c == '\n') {
                break;
            }
        }
        else {
            std::this_thread::yield();
        }
    }
    buf[read_length] = '\0';
    if (plength != nullptr) {
        (*plength) = read_length;
    }

    return true;
}

bool StandardIo::read_with_timeout(void* buf, size_t bufsize, size_t* pread, int32_t timeout) {
    if ((buf == nullptr) || (bufsize == 0) || (pread == nullptr)) {
        return false;
    }

    bool retval = false;
    uint8_t* wp = static_cast<uint8_t*>(buf);
    size_t read_length = 0;
    size_t left = bufsize;
    ULONGLONG begin = GetTickCount64();
    while (!is_input_EOF() // �I�[���o���Ă��Ȃ��H
        && (left > 0)) { // �ǂݏo���c�ʂ�����H
        size_t length = 0;
        retval = read(wp, left, &length);
        if (retval) {
            wp += length;
            read_length += length;
            left -= length;
        }
        else {
            break;
        }

        if ((timeout >= 0) // �^�C���A�E�g���Ԃ�0�ȏ�H
            && ((GetTickCount64() - begin) >= timeout)) { // �o�ߎ��Ԃ��^�C���A�E�g���Ԉȏ�H
            break;
        }
    }
    (*pread) = read_length;
    return retval;
}

bool StandardIo::read(void* buf, size_t bufsize, size_t* pread) {
    if ((buf == nullptr) || (bufsize == 0) || (pread == nullptr)) {
        return false;
    }

    uint8_t* wp = static_cast<uint8_t*>(buf);

    size_t read_length = 0;
    if (!m_input_data.empty()) {
        std::lock_guard<std::mutex> lock(m_input_lock);
        while ((read_length < bufsize) // �ǂݏo��������bufsize�����H
            && !m_input_data.empty()) { // �f�[�^����łȂ��H
            (*wp) = m_input_data.front();
            m_input_data.pop();
            read_length++;
            wp++;
        }
    }
    (*pread) = read_length;
    
    return true;
}


bool StandardIo::print(std_io* pstdio, const std::string& str) {
    if ((*pstdio).is_console) {
        const std::string outstr = replace_CRLF(str);
        const char* cstr = outstr.c_str();
        size_t length = strlen(cstr);
        return write(pstdio, cstr, length);
    }
    else {
        const char* cstr = str.c_str();
        size_t length = strlen(cstr);
        return write(pstdio, cstr, length);
    }
}

bool StandardIo::write(std_io* pstdio, const void* data, size_t length, size_t * pwritten) {
    if (pstdio->handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    const uint8_t* rp = reinterpret_cast<const uint8_t*>(data);
    auto left = static_cast<DWORD>(length);

    while (left > 0) {
        DWORD transferred = 0;
        if (WriteFile((*pstdio).handle, rp, left, &transferred, nullptr)) {
            rp += transferred;
            left -= transferred;
        }
        else {
            break;
        }
    }
    if (pwritten != nullptr) {
        (*pwritten) = length - left;
    }
    return (left == 0);
}

std::string StandardIo::replace_CRLF(const std::string& str) {
    std::ostringstream oss;

    char prev = '\0';
    for (auto c : str) {
        if ((c != '\n') && (prev == '\r')) { // CR�݂̂������H
            oss << '\n'; // LF��ǉ����� CR+LF �ɂ���B
        }
        else if ((c == '\n') && (prev != '\r')) { // LF�̂݁H
            oss << '\r'; // CR��ǉ����� CR+LF �ɂ���B
        }
        oss << c;
        prev = c;
    }
    if (prev == '\r') { // ������CR�̂݁H
        oss << '\n'; // LF��ǉ����� CR+LF �ɂ���B
    }
    return oss.str();
}


void StandardIo::receiver_thread_proc(void) {
    // Note : �W�����͂��L���łȂ��ꍇ�ɂ͋N������Ȃ��̂ŁA
    //        ReadFile()�Ăяo���O��is_input_valid()�͕s�v�B
    uint8_t buf[256];
    while (!Terminated) {
        DWORD read_len = 0;
        if (m_input_data.size() < m_max_read_length) {
            if (m_input.is_console) {
                read_from_console();
            }
            else {
                read_from_pipe();
            }

            size_t io_length = min(sizeof(buf), m_max_read_length - m_input_data.size());
        }
        else {
            std::this_thread::yield();
        }
    }
}
void StandardIo::read_from_console(void) {
    char buf[1];
    DWORD read_len = 0;
    if (!ReadFile(m_input.handle, buf, 1, &read_len, nullptr)) {
        return;
    }

    if (read_len == 0) { // �ǂݏo����������0�H
        Terminated = true;
        return;
    }

    if (is_line_input_mode()) {
        std::lock_guard<std::mutex> lock(m_input_lock);
        m_input_data.push(buf[0]);
    } else {
        uint8_t write_data[4]; // �o�͕�����
        uint32_t write_len;

        uint8_t c = buf[0];
        if ((c == '\r')) { // CR�̂݁H
            write_data[0] = '\r';
            write_data[1] = '\n';
            write_len = 2;
        }
        else if ((c == '\n') && (m_prev_input_data != '\r')) { // LF�̂݁H
            write_data[0] = '\r';
            write_data[1] = c;
            write_len = 2;
        }
        else if (c == 0x04) {
            // EOT���m(Ctrl+D)
            Terminated = true;
            write_len = 0;
        }
        else {
            write_data[0] = c;
            write_len = 1;
        }

        std::lock_guard<std::mutex> lock(m_input_lock);
        for (uint32_t i = 0; i < write_len; i++) {
            m_input_data.push(write_data[i]);
        }
    }

    return;
}
void StandardIo::read_from_pipe(void) {
    uint8_t buf[256];
    DWORD io_length = static_cast<DWORD>(min(sizeof(buf), m_max_read_length - m_input_data.size()));
    DWORD read_len = 0;
    if (ReadFile(m_input.handle, buf, io_length, &read_len, nullptr)) {
        std::lock_guard<std::mutex> lock(m_input_lock);
        for (size_t i = 0; i < read_len; i++) {
            m_input_data.push(buf[i]);
        }
    }
    else {
        auto err = GetLastError();
        if (err == ERROR_BROKEN_PIPE) {
            // ���_�C���N�g���ꂽ���͂̏ꍇ�A
            // �I�[�ɒB����� ERROR_BROKEN_PIPE���Ԃ�悤�ɂȂ�B
            Terminated = true;
        }
    }

}

BOOL StandardIo::console_handler_proc(DWORD event) {
    BOOL retval;
    switch (event) {
    case CTRL_BREAK_EVENT: // Ctrl-Break
    case CTRL_CLOSE_EVENT: // �R���\�[���N���[�Y
        Terminated = true;
        retval = TRUE;
        break;
    default:
        retval = FALSE;
        break;
    }
    return retval;
}