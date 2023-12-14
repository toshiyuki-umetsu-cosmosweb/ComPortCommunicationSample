#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <windows.h>


class SerialPort
{
public:
    /**
     * �V���A���|�[�g�ꗗ��񋓂���B
     *
     * @param plist �V���A���|�[�g���ꗗ���擾���郊�X�g
     * @retval true ����
     * @retval false ���s
     */
    static bool enumerate_ports(std::vector<std::string>* plist);
    /**
     * StopBit 1bit
     */
    static const uint32_t StopBitsOne;
    /**
     * StopBit 1.5bit
     */
    static const uint32_t StopBitsOne5;
    /**
     * StopBit 2bit
     */
    static const uint32_t StopBitsTwo;

    /**
     * �p���e�B����
     */
    static const uint32_t ParityNone;
    /**
     * �����p���e�B
     */
    static const uint32_t ParityEven;
    /**
     * ��p���e�B
     */
    static const uint32_t ParityOdd;
    /**
     * CTS�Ď�����(�ڑ���̏o��RTS�M���𖳎�����)
     */
    static const uint32_t CtsFlowDisable;
    /**
     * CTS�Ď��L��(�ڑ���̏o�͂���RTS�M�����݂đ��M���䂷��)
     */
    static const uint32_t CtsFlowEnable;

    /**
     * RTS L�Œ� (��M�s�ŕێ�)
     */
    static const uint32_t RtsControlDisable;
    /**
     * RTS H�Œ� (��M�ŕێ�)
     */
    static const uint32_t RtsControlEnable;
    /**
     * RTS�M���L��
     */
    static const uint32_t RtsControlHandShake;
    /**
     * RTS�M�����M�D��(���M�f�[�^������Ƃ���RTS��L�ɂ���)
     */
    static const uint32_t RtsControlToggle;
    /**
     * BREAK���o�G���[
     */
    static const uint32_t ErrorBreak;
    /**
     * �t���[�~���O�G���[���o
     */
    static const uint32_t ErrorFrame;
    /**
     * �I�[�o�[�����G���[����
     */
    static const uint32_t ErrorOverrRun;
    /**
     * ��M�I�[�o�[�t���[
     */
    static const uint32_t ErrorReceiveOverflow;
    /**
     * ��M�p���e�B�G���[
     */
    static const uint32_t ErrorReceiveParity;

    /**
     * �G���[�n���h���^
     * 
     * @param errors �G���[(ErrorBreak,ErrorFrame,ErrorOverRun,
     *                      ErrorReceiveOverflow,ErrorReceiveParity�̂����ꂩ)
     */
    typedef std::function<void(uint32_t)> error_handler_t;

    /**
     * �R���X�g���N�^
     * 
     * @param port_name �V���A���|�[�g��("COM1"�Ȃ�)
     */
    explicit SerialPort(const std::string& port_name);

    /**
     * �R���X�g���N�^
     * 
     * @param port_name �V���A���|�[�g��("COM1"�Ȃ�)
     * @param ref_port �ݒ�������p���|�[�g
     */
    SerialPort(const std::string& port_name, const SerialPort& ref_port);

    /**
     * �f�X�g���N�^
     */
    ~SerialPort(void);

    /**
     * �I�[�v���ς݂��ǂ������擾����B
     *
     * @retval true �I�[�v���ς�
     * @retval false �I�[�v�����Ă��Ȃ�
     */
    bool is_opened(void) const noexcept { return m_port_handle != INVALID_HANDLE_VALUE; }
    /**
     * �V���A���|�[�g���I�[�v������
     */
    void open(void);
    /**
     * �V���A���|�[�g���N���[�Y����
     */
    void close(void);

    /**
     * �{�[���[�g��ݒ肷��B
     * 
     * @param baudrate �{�[���[�g[bps]
     */
    void set_baudrate(uint32_t baudrate) {
        if (baudrate != m_baudrate) {
            m_baudrate = baudrate;
            apply_settings();
        }
    }
    /**
     * �{�[���[�g���擾����B
     * @retval �{�[���[�g[bps]
     */
    uint32_t get_baudrate(void) const noexcept { return m_baudrate; }
    /**
     * �f�[�^�r�b�g����ݒ肷��B
     * 
     * @param databits �f�[�^�r�b�g��
     */
    void set_databits(uint8_t databits) {
        if (((databits == 7) || (databits == 8)) // �f�[�^�r�b�g�� 7or8
            && (databits != m_databits)) { // �ݒ肪�Ⴄ�H
            m_databits = databits;
            apply_settings();
        }
    }
    /**
     * �f�[�^�r�b�g���𓾂�
     * 
     * @retval �f�[�^�r�b�g��
     */
    uint8_t get_databits(void) const noexcept { return m_databits; }
    /**
     * �p���e�B�ݒ������B
     * 
     * @param parity �p���e�B�ݒ�(ParityNone, ParityEven, ParityOdd�̂����ꂩ)
     */
    void set_parity(uint32_t parity) {
        if (((parity == ParityNone) || (parity == ParityEven) || (parity == ParityOdd)) // parity�ݒ�l�͐������H
            && (parity != m_parity)) { // �ݒ�l���Ⴄ�H
            m_parity = parity;
            apply_settings();
        }
    }
    /**
     * �p���e�B�ݒ�𓾂�B
     * 
     * @retval �p���e�B�ݒ�(ParityNone, ParityEven, ParityOdd�̂����ꂩ)
     */
    uint32_t get_parity(void) const noexcept { return m_parity; }

    /**
     *  �X�g�b�v�r�b�g�ݒ������B
     * 
     * @param stopbits �X�g�b�v�r�b�g�ݒ�(StopBitsOne, StopBitsOne5, StopBitsTwo�̂����ꂩ)
     */
    void set_stopbits(uint32_t stopbits) {
        if (((stopbits == StopBitsOne) || (stopbits == StopBitsOne5) || (stopbits == StopBitsTwo)) // stopbits�ݒ�͐������H
            && (stopbits != m_stopbits)) { // �ݒ�l���Ⴄ�H
            m_stopbits = stopbits;
            apply_settings();
        }
    }
    /**
     * �X�g�b�v�r�b�g�ݒ�𓾂�B
     * 
     * @retval �X�g�b�v�r�b�g�ݒ�(StopBitsOne, StopBitsOne5, StopBitsTwo�̂����ꂩ)
     */
    uint32_t get_stopbits(void) const noexcept { return m_stopbits; }

    /**
     * CTS�����ݒ肷��B
     * 
     * @param cts_flow CTS����(CtsFlowDisable, CtsFlowEnable�̂����ꂩ)
     */
    void set_cts_flow(uint32_t cts_flow) {
        if (((cts_flow == CtsFlowDisable) || (cts_flow == CtsFlowEnable))
            && (cts_flow != m_cts_flow)) {
            m_cts_flow = cts_flow;
            apply_settings();
        }
    }
    /**
     * CTS������擾����B
     * 
     * @retval CTS����(CtsFlowDisable, CtsFlowEnable�̂����ꂩ)
     */
    uint32_t get_cts_flow(void) const noexcept { return m_cts_flow; }
    /**
     * RTS�����ݒ肷��B
     * 
     * @param rts_control RTS����(RtsControlDisable, RtsControlEnable, RtsControlHandShake, RtsControlToggle)�̂����ꂩ
     */
    void set_rts_control(uint32_t rts_control) {
        if (((rts_control == RtsControlDisable) || (rts_control == RtsControlEnable) 
                || (rts_control == RtsControlHandShake) || (rts_control == RtsControlToggle)) // �ݒ�l�͐������H
            && (rts_control != m_rts_control)) { // �ݒ�l���Ⴄ?
            m_rts_control = rts_control;
            apply_settings();
        }
    }
    /**
     * RTS������擾����B
     * 
     * @retval RTS����(RtsControlDisable, RtsControlEnable, RtsControlHandShake, RtsControlToggle)�̂����ꂩ
     */
    uint32_t get_rts_control(void) const noexcept { return m_rts_control; }

    /**
     * ���M����B
     * ���M�������邩�Atimeout_millis���Ԍo�߂���܂ŌĂяo�������u���b�N����B
     * 
     * @param data ���M�f�[�^
     * @param length ���M�f�[�^�T�C�Y
     * @param timeout_millis �^�C���A�E�g����[�~���b] �����ɂ���Ɖi���ɑ҂B
     * @retval -1 �G���[�����������ꍇ
     * @retval 0�ȏ�̒l �ǂݏo�����o�C�g��
     */
    int send(const uint8_t* data, uint32_t length, int timeout_millis = -1);
    /**
     * ��M����B
     * ��M�������邩�Atimeout_millis���Ԍo�߂���܂ŌĂяo�������u���b�N����B
     * 
     * @param buf �ǂݏo���f�[�^���i�[����o�b�t�@
     * @param bufsize �o�b�t�@�T�C�Y
     * @param timeout_millis �^�C���A�E�g����[�~���b] �����ɂ���Ɖi���ɑ҂B
     * @retval -1 �G���[�����������ꍇ
     * @retval 0�ȏ�̒l �ǂݏo�����o�C�g��
     */
    int receive(uint8_t* buf, uint32_t bufsize, int timeout_millis = -1);
    /**
     * �G���[�n���h����ǉ�����B
     * 
     * @param handler �ǉ�����G���[�n���h��
     */
    void set_error_handler(const error_handler_t& handler) {
        if (m_error_handler) {
            m_error_handler = handler;
        }
        else {
            m_error_handler = nullptr;
        }
    }

private:
    HANDLE m_port_handle; // �V���A���|�[�g�C���X�^���X�̃n���h��
    std::string m_port_name; // �V���A���|�[�g��
    uint32_t m_baudrate; // �{�[���[�g
    uint8_t m_databits; // �f�[�^�r�b�g
    uint32_t m_parity; // �p���e�B
    uint32_t m_stopbits; // �X�g�b�v�r�b�g
    uint32_t m_cts_flow; // CTS�L���ݒ�
    uint32_t m_rts_control; // RTS����ݒ�
    error_handler_t m_error_handler; // �G���[�n���h��

    /**
     * �ݒ��K�p����B
     * �V���A���|�[�g�̃C���X�^���X���I�[�v������Ă��Ȃ��ꍇ�ɂ͉������Ȃ��B
     */
    void apply_settings(void);
    /**
     * I/O�҂�������B
     * 
     * @param req �v��
     * @param timeout_millis �^�C���A�E�g����[�~���b] �����ɂ���Ɖi���ɑ҂B
     * @retval -1 ���s
     * @retval 0�ȏ�̒l ����M�����o�C�g��
     */
    int wait_io(LPOVERLAPPED req, int timeout_millis);
    /**
     * �G���[��������
     * 
     * @param error �G���[
     */
    void handle_errors(DWORD errors) const {
        if (m_error_handler) {
            m_error_handler(errors);
        }
    }

    // ����R���X�g���N�^�͎g�p�ł��Ȃ��B
    SerialPort(const SerialPort& port) = delete;
    // ������Z�q�͎g�p�ł��Ȃ�
    SerialPort& operator=(const SerialPort& port) = delete;
};

