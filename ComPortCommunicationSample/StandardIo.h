#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <vector>
#include <string>
#include <queue>
#include <mutex>

/**
 * �W�����o�̓��b�p�[
 * 
 * @note
 * std::cin ����� std::cout���g���������ǂ��Ǝv���B
 */
class StandardIo
{
public:
    static StandardIo& instance(void);

    ~StandardIo(void);

    /**
     * �s�P�ʓ��̓��[�h��ݒ肷��B
     * 
     * @param is_enabled �s�P�ʓ��̓��[�h�ɂ���ꍇ�ɂ�true, ����ȊO��false.
     * @retval true ����
     * @retval false ���s
     */
    bool set_line_input_mode(bool is_enable) {
        return modify_console_mode(&m_input, LineInputModeFunctions, is_enable);
    }

    /**
     * �s�P�ʓ��̓��[�h���ǂ����𓾂�
     * 
     * @retval true �s�P�ʓ��̓��[�h
     * @retval false �����P�ʓ��̓��[�h
     */
    bool is_line_input_mode(void) const noexcept {
        return ((m_input.mode & LineInputModeFunctions) == LineInputModeFunctions) ? true : false;
    }

    /**
     * �ǂݏo���o�b�t�@�̃T�C�Y��ݒ肷��B
     * 
     * @param length ����
     */
    bool set_max_read_length(uint32_t length) {
        if (length > 0) {
            m_max_read_length = length;
            return true;
        }
        else {
            return false;
        }
    }
    /**
     * �ǂݏo���o�b�t�@�̃T�C�Y���擾����B
     * 
     * @retval �o�b�t�@�T�C�Y
     */
    uint32_t get_max_read_length(void) const noexcept {
        return m_max_read_length;
    }

    /**
     * �ǂݏo���o�b�t�@�ɂ��܂��Ă���f�[�^�ʂ��擾����B
     * 
     * @retval �f�[�^��
     */
    uint32_t get_read_data_length(void) const noexcept {
        return static_cast<uint32_t>(m_input_data.size());
    }

    /**
     * ���͂��I�[�������ǂ����𔻒肷��B
     * 
     * @retval true �I�[���Ă���ꍇ
     * @retval false �I�[���Ă��Ȃ��ꍇ
     */
    bool is_input_EOF(void) const noexcept { return Terminated; }
    /**
     * �_���^�ւ̃L���X�g���Z�q
     * 
     * @retval true ���͂��L���ȏꍇ
     * @retval false ���͂������ȏꍇ
     */
    operator bool() const noexcept {
        return is_input_valid() && !is_input_EOF();
    }
    /**
     * �ے艉�Z�q
     * 
     * @retval true ���͂������ȏꍇ
     * @retval false ���͂��L���ȏꍇ
     */
    bool operator!() const noexcept {
        return !this->operator bool();
    }

    /**
     * ���͂��L�����ǂ����B
     * �R���\�[���ɐڑ�����Ă��邩�A���_�C���N�g����Ă���ꍇ��true�B
     * �ǂ����̎q�v���Z�X�Ƃ��ċN������āA���_�C���N�g����Ă��Ȃ��ꍇ�ɂ�false.
     * 
     * @retval true �L��
     * @retval false ����
     */
    bool is_input_valid(void) const noexcept {
        return (m_input.handle != INVALID_HANDLE_VALUE) ? true : false;
    }
    /**
     * �W���o�͂��L�����ǂ����B
     * �R���\�[���ɐڑ�����Ă��邩�A���_�C���N�g����Ă���ꍇ��true�B
     * �ǂ����̎q�v���Z�X�Ƃ��ċN������āA���_�C���N�g����Ă��Ȃ��ꍇ�ɂ�false.
     *
     * @retval true �L��
     * @retval false ����
     */
    bool is_output_valid(void) const noexcept {
        return (m_output.handle != INVALID_HANDLE_VALUE) ? true : false;
    }
    /**
     * �W���G���[�o�͂��L�����ǂ����B
     * �R���\�[���ɐڑ�����Ă��邩�A���_�C���N�g����Ă���ꍇ��true�B
     * �ǂ����̎q�v���Z�X�Ƃ��ċN������āA���_�C���N�g����Ă��Ȃ��ꍇ�ɂ�false.
     *
     * @retval true �L��
     * @retval false ����
     */
    bool is_error_valid(void) const noexcept {
        return (m_error.handle != INVALID_HANDLE_VALUE) ? true : false;
    }

    /**
     * 1�s�ǂݏo���B
     * ���͂��I�[���Ă���ꍇ�ɂ͋󕶎��񂪕Ԃ�B
     * ���s�R�[�h�����o����O�ɓ��͂��I�[�����ꍇ�ɂ́A�r���܂ł̕����񂪕Ԃ�B
     * �Ԃ�������͉��s�R�[�h���܂ށB
     * 
     * @retval �s
     */
    std::string read_line(void);
    /**
     * ���s�R�[�h�����o���邩�Abufsize - 1�����ǂނ܂œǂݏo���B
     * 
     * @param buf �o�b�t�@(���Ȃ��Ƃ�2�o�C�g�ȏ�i�[�ł���悤�ȗ̈�Ƃ��邱��)
     * @param bufsize �o�b�t�@�T�C�Y
     * @param plength �ǂݏo�����������i�[����ϐ�(�s�v�ȏꍇ�ɂ�nullptr)
     * @retval true ����
     * @retval false ���s(�s���ȃp�����[�^���n���ꂽ�ꍇ)
     */
    bool read_line(char* buf, size_t bufsize, size_t* plength = nullptr);
    /**
     * ���̓o�b�t�@����ő��bufsize�����ǂݏo���B
     * timeout�Ŏw�肵�����Ԃ����ҋ@����B
     * 
     * @param buf �o�b�t�@
     * @param bufsize �o�b�t�@�T�C�Y
     * @param pread �ǂݏo�����������i�[����ϐ�
     * @param timeout �^�C���A�E�g����[�~���b](�����ɂ���Ɖi���ɑ҂�)
     * @retval true ����
     * @retval false ���s
     */
    bool read_with_timeout(void* buf, size_t bufsize, size_t* pread, int32_t timeout);
    /**
     * ���̓o�b�t�@����ő��bufsize�����ǂݏo���B
     * �{�C���^�t�F�[�X�͓��͑҂������Ȃ��B
     * 
     * @param buf �o�b�t�@
     * @param bufsize �o�b�t�@�T�C�Y
     * @param pread �ǂݏo�����������i�[����ϐ�
     * @retval true ����
     * @retval false ���s
     */
    bool read(void* buf, size_t bufsize, size_t* pread);

    /**
     * ������������w��ŏo�͂���B
     * 
     * @param fmt �t�H�[�}�b�g
     * @param args �p�����[�^
     * @retval true ����
     * @retval false ���s
     */
    template <typename ... Args>
    bool print(const char* fmt, Args ... args) {
        if (fmt != nullptr) {
            size_t len = snprintf(nullptr, 0, fmt, args ...);
            std::vector<char> buf(len + 1);
            snprintf(&buf[0], len + 1, fmt, args ...);
            std::string msg(&buf[0], &buf[0] + len);
            return print(&m_output, msg);
        }
        else {
            return true;
        }
    }

    /**
     * ��������o�͂���B
     * 
     * @param str ������
     * @retval true ����
     * @retval false ���s
     */
    bool print(const std::string& str) {
        return print(&m_output, str);
    }

    /**
     * �������o�͂���B
     * 
     * @param c ����
     * @retval true ����
     * @retval false ���s
     */
    bool print(char c) {
        char buf[1];
        buf[0] = c;
        return write(&m_output, buf, 1);
    }
    /**
     * ������������w��ŕW���G���[�o�͂ɏo�͂���B
     *
     * @param fmt �t�H�[�}�b�g
     * @param args �p�����[�^
     * @retval true ����
     * @retval false ���s
     */
    template <typename ... Args>
    bool print_err(const char* fmt, Args ... args) {
        if (fmt != nullptr) {
            size_t len = snprintf(nullptr, 0, fmt, args ...);
            std::vector<char> buf(len + 1);
            snprintf(&buf[0], len + 1, fmt, args ...);
            std::string msg(&buf[0], &buf[0] + len);
            return print(&m_error, msg);
        }
        else {
            return true;
        }
    }

    /**
     * �������W���G���[�o�͂ɏo�͂���B
     *
     * @param str ������
     * @retval true ����
     * @retval false ���s
     */
    bool print_err(const std::string& str) {
        return print(&m_error, str);
    }
    /**
     * ������W���G���[�o�͂ɏo�͂���B
     *
     * @param c ����
     * @retval true ����
     * @retval false ���s
     */
    bool print_err(char c) {
        char buf[1];
        buf[0] = c;
        return write(&m_error, buf, 1);
    }

    /**
     * �f�[�^���o�͂���B
     * 
     * @param data �f�[�^�̃A�h���X
     * @param length �f�[�^�̒���
     * @param pwritten �o�͂����f�[�^�����󂯎��ϐ�(�s�v�ȏꍇ��nullptr)
     * @retval true ����
     * @retval false ���s
     */
    bool write(const void* data, size_t length, size_t *pwritten = nullptr) {
        if ((data == nullptr)) {
            return false;
        }
        else {
            return write(&m_output, data, length, pwritten);
        }
    }

    /**
     * �f�[�^��W���G���[�o�͂ɏo�͂���B
     * 
     * @param data �f�[�^�̃A�h���X
     * @param length �f�[�^�̒���
     * @param pwritten �o�͂����f�[�^�����󂯎��ϐ�(�s�v�ȏꍇ��nullptr)
     * @retval true ����
     * @retval false ���s
     */
    bool write_err(const void* data, size_t length, size_t *pwritten) {
        if (data == nullptr) {
            return false;
        }
        else {
            return write(&m_error, data, length, pwritten);
        }

    }

private:
    struct std_io {
        HANDLE handle; // �n���h��
        bool is_console; // �R���\�[�����ǂ���
        DWORD mode; // �R���\�[�����[�h
        std_io(void) : handle(INVALID_HANDLE_VALUE), is_console(false), mode(0) { }
    };
    bool m_initialized; // �������������ǂ����̃t���O
    std_io m_input; // �W������
    std_io m_output; // �W���o��
    std_io m_error; // �W���G���[�o��
    std::queue<uint8_t> m_input_data; // ���̓f�[�^
    std::mutex m_input_lock; // ���̓��b�N
    uint32_t m_max_read_length; // �ǂݏo���o�b�t�@�T�C�Y
    char m_prev_input_data; // �O����͕���
    static bool Terminated; // �I�[���m������
    static const DWORD LineInputModeFunctions; // �s�P�ʓ��̓��[�h�@�\

    /**
     * �R���X�g���N�^
     */
    StandardIo(void);
    /**
     * ����������B
     * 
     * @retval true ����
     * @retval false ���s
     */
    bool init(void);
    /**
     * �������������ǂ���
     * 
     * @retval true �������ς�
     * @retval false ��������
     */
    bool is_initialized(void) const noexcept { return m_initialized; }

    /**
     * �n���h��������������B
     * 
     * @param pstdio I/O�I�u�W�F�N�g
     * @param handle_type �n���h���^�C�v
     */
    void init_std_handle(std_io* pstdio, DWORD handle_type);
    /**
     * �R���\�[�����[�h��ݒ肷��B
     * 
     * @param pstdio I/O�I�u�W�F�N�g
     * @param functions �Ώۂ̋@�\
     * @param is_enabled �L���ɂ���ꍇ��true, �����ɂ���ꍇ��false.
     */
    bool modify_console_mode(std_io* pstdio, DWORD functions, bool is_enabled);
    /**
     * �o�͂���
     * 
     * @param pstdio I/O�I�u�W�F�N�g
     * @param str �o�͕�����
     * @retval true ����
     * @retval false ���s
     */
    bool print(std_io* pstdio, const std::string &str);
    /**
     * �o�͂���
     *
     * @param pstdio I/O�I�u�W�F�N�g
     * @param data ���M�f�[�^�|�C���^
     * @param length ����
     * @param pwritten �������񂾒���(�s�v�ȏꍇ�ɂ�nullptr��n��)
     * @retval true ����
     * @retval false ���s
     */
    bool write(std_io* pstdio, const void* data, size_t length, size_t* pwritten = nullptr);

    /**
     * ���s�R�[�h��CR+LF�ɑ�����
     * 
     * @param str ������
     * @retval �u������������
     */
    std::string replace_CRLF(const std::string& str);

    /**
     * ��M�X���b�h����
     */
    void receiver_thread_proc(void);
    /**
     * �R���\�[������̓��͓ǂݏo�����s���B
     */
    void read_from_console(void);
    /**
     * �p�C�v����̓��͓ǂݏo�����s���B
     */
    void read_from_pipe(void);

    /**
     * �R���\�[���n���h��
     * 
     * @param event �C�x���g
     */
    static BOOL console_handler_proc(DWORD event);

    StandardIo(const StandardIo& io) = delete;
    StandardIo& operator=(const StandardIo& io) = delete;
};

