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
     * シリアルポート一覧を列挙する。
     *
     * @param plist シリアルポート名一覧を取得するリスト
     * @retval true 成功
     * @retval false 失敗
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
     * パリティ無し
     */
    static const uint32_t ParityNone;
    /**
     * 偶数パリティ
     */
    static const uint32_t ParityEven;
    /**
     * 奇数パリティ
     */
    static const uint32_t ParityOdd;
    /**
     * CTS監視無効(接続先の出すRTS信号を無視する)
     */
    static const uint32_t CtsFlowDisable;
    /**
     * CTS監視有効(接続先の出力するRTS信号をみて送信制御する)
     */
    static const uint32_t CtsFlowEnable;

    /**
     * RTS L固定 (受信不可で保持)
     */
    static const uint32_t RtsControlDisable;
    /**
     * RTS H固定 (受信可で保持)
     */
    static const uint32_t RtsControlEnable;
    /**
     * RTS信号有効
     */
    static const uint32_t RtsControlHandShake;
    /**
     * RTS信号送信優先(送信データがあるときはRTSをLにする)
     */
    static const uint32_t RtsControlToggle;
    /**
     * BREAK検出エラー
     */
    static const uint32_t ErrorBreak;
    /**
     * フレーミングエラー検出
     */
    static const uint32_t ErrorFrame;
    /**
     * オーバーランエラー発生
     */
    static const uint32_t ErrorOverrRun;
    /**
     * 受信オーバーフロー
     */
    static const uint32_t ErrorReceiveOverflow;
    /**
     * 受信パリティエラー
     */
    static const uint32_t ErrorReceiveParity;

    /**
     * エラーハンドラ型
     * 
     * @param errors エラー(ErrorBreak,ErrorFrame,ErrorOverRun,
     *                      ErrorReceiveOverflow,ErrorReceiveParityのいずれか)
     */
    typedef std::function<void(uint32_t)> error_handler_t;

    /**
     * コンストラクタ
     * 
     * @param port_name シリアルポート名("COM1"など)
     */
    explicit SerialPort(const std::string& port_name);

    /**
     * コンストラクタ
     * 
     * @param port_name シリアルポート名("COM1"など)
     * @param ref_port 設定を引き継ぐポート
     */
    SerialPort(const std::string& port_name, const SerialPort& ref_port);

    /**
     * デストラクタ
     */
    ~SerialPort(void);

    /**
     * オープン済みかどうかを取得する。
     *
     * @retval true オープン済み
     * @retval false オープンしていない
     */
    bool is_opened(void) const noexcept { return m_port_handle != INVALID_HANDLE_VALUE; }
    /**
     * シリアルポートをオープンする
     */
    void open(void);
    /**
     * シリアルポートをクローズする
     */
    void close(void);

    /**
     * ボーレートを設定する。
     * 
     * @param baudrate ボーレート[bps]
     */
    void set_baudrate(uint32_t baudrate) {
        if (baudrate != m_baudrate) {
            m_baudrate = baudrate;
            apply_settings();
        }
    }
    /**
     * ボーレートを取得する。
     * @retval ボーレート[bps]
     */
    uint32_t get_baudrate(void) const noexcept { return m_baudrate; }
    /**
     * データビット数を設定する。
     * 
     * @param databits データビット数
     */
    void set_databits(uint8_t databits) {
        if (((databits == 7) || (databits == 8)) // データビットが 7or8
            && (databits != m_databits)) { // 設定が違う？
            m_databits = databits;
            apply_settings();
        }
    }
    /**
     * データビット数を得る
     * 
     * @retval データビット数
     */
    uint8_t get_databits(void) const noexcept { return m_databits; }
    /**
     * パリティ設定をする。
     * 
     * @param parity パリティ設定(ParityNone, ParityEven, ParityOddのいずれか)
     */
    void set_parity(uint32_t parity) {
        if (((parity == ParityNone) || (parity == ParityEven) || (parity == ParityOdd)) // parity設定値は正しい？
            && (parity != m_parity)) { // 設定値が違う？
            m_parity = parity;
            apply_settings();
        }
    }
    /**
     * パリティ設定を得る。
     * 
     * @retval パリティ設定(ParityNone, ParityEven, ParityOddのいずれか)
     */
    uint32_t get_parity(void) const noexcept { return m_parity; }

    /**
     *  ストップビット設定をする。
     * 
     * @param stopbits ストップビット設定(StopBitsOne, StopBitsOne5, StopBitsTwoのいずれか)
     */
    void set_stopbits(uint32_t stopbits) {
        if (((stopbits == StopBitsOne) || (stopbits == StopBitsOne5) || (stopbits == StopBitsTwo)) // stopbits設定は正しい？
            && (stopbits != m_stopbits)) { // 設定値が違う？
            m_stopbits = stopbits;
            apply_settings();
        }
    }
    /**
     * ストップビット設定を得る。
     * 
     * @retval ストップビット設定(StopBitsOne, StopBitsOne5, StopBitsTwoのいずれか)
     */
    uint32_t get_stopbits(void) const noexcept { return m_stopbits; }

    /**
     * CTS制御を設定する。
     * 
     * @param cts_flow CTS制御(CtsFlowDisable, CtsFlowEnableのいずれか)
     */
    void set_cts_flow(uint32_t cts_flow) {
        if (((cts_flow == CtsFlowDisable) || (cts_flow == CtsFlowEnable))
            && (cts_flow != m_cts_flow)) {
            m_cts_flow = cts_flow;
            apply_settings();
        }
    }
    /**
     * CTS制御を取得する。
     * 
     * @retval CTS制御(CtsFlowDisable, CtsFlowEnableのいずれか)
     */
    uint32_t get_cts_flow(void) const noexcept { return m_cts_flow; }
    /**
     * RTS制御を設定する。
     * 
     * @param rts_control RTS制御(RtsControlDisable, RtsControlEnable, RtsControlHandShake, RtsControlToggle)のいずれか
     */
    void set_rts_control(uint32_t rts_control) {
        if (((rts_control == RtsControlDisable) || (rts_control == RtsControlEnable) 
                || (rts_control == RtsControlHandShake) || (rts_control == RtsControlToggle)) // 設定値は正しい？
            && (rts_control != m_rts_control)) { // 設定値が違う?
            m_rts_control = rts_control;
            apply_settings();
        }
    }
    /**
     * RTS制御を取得する。
     * 
     * @retval RTS制御(RtsControlDisable, RtsControlEnable, RtsControlHandShake, RtsControlToggle)のいずれか
     */
    uint32_t get_rts_control(void) const noexcept { return m_rts_control; }

    /**
     * 送信する。
     * 送信完了するか、timeout_millis時間経過するまで呼び出し元をブロックする。
     * 
     * @param data 送信データ
     * @param length 送信データサイズ
     * @param timeout_millis タイムアウト時間[ミリ秒] 負数にすると永遠に待つ。
     * @retval -1 エラーが発生した場合
     * @retval 0以上の値 読み出したバイト数
     */
    int send(const uint8_t* data, uint32_t length, int timeout_millis = -1);
    /**
     * 受信する。
     * 受信完了するか、timeout_millis時間経過するまで呼び出し元をブロックする。
     * 
     * @param buf 読み出しデータを格納するバッファ
     * @param bufsize バッファサイズ
     * @param timeout_millis タイムアウト時間[ミリ秒] 負数にすると永遠に待つ。
     * @retval -1 エラーが発生した場合
     * @retval 0以上の値 読み出したバイト数
     */
    int receive(uint8_t* buf, uint32_t bufsize, int timeout_millis = -1);
    /**
     * エラーハンドラを追加する。
     * 
     * @param handler 追加するエラーハンドラ
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
    HANDLE m_port_handle; // シリアルポートインスタンスのハンドル
    std::string m_port_name; // シリアルポート名
    uint32_t m_baudrate; // ボーレート
    uint8_t m_databits; // データビット
    uint32_t m_parity; // パリティ
    uint32_t m_stopbits; // ストップビット
    uint32_t m_cts_flow; // CTS有効設定
    uint32_t m_rts_control; // RTS制御設定
    error_handler_t m_error_handler; // エラーハンドラ

    /**
     * 設定を適用する。
     * シリアルポートのインスタンスがオープンされていない場合には何もしない。
     */
    void apply_settings(void);
    /**
     * I/O待ちをする。
     * 
     * @param req 要求
     * @param timeout_millis タイムアウト時間[ミリ秒] 負数にすると永遠に待つ。
     * @retval -1 失敗
     * @retval 0以上の値 送受信したバイト数
     */
    int wait_io(LPOVERLAPPED req, int timeout_millis);
    /**
     * エラー処理する
     * 
     * @param error エラー
     */
    void handle_errors(DWORD errors) const {
        if (m_error_handler) {
            m_error_handler(errors);
        }
    }

    // 代入コンストラクタは使用できない。
    SerialPort(const SerialPort& port) = delete;
    // 代入演算子は使用できない
    SerialPort& operator=(const SerialPort& port) = delete;
};

