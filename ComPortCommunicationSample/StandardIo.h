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
 * 標準入出力ラッパー
 * 
 * @note
 * std::cin および std::coutを使った方が良いと思う。
 */
class StandardIo
{
public:
    static StandardIo& instance(void);

    ~StandardIo(void);

    /**
     * 行単位入力モードを設定する。
     * 
     * @param is_enabled 行単位入力モードにする場合にはtrue, それ以外はfalse.
     * @retval true 成功
     * @retval false 失敗
     */
    bool set_line_input_mode(bool is_enable) {
        return modify_console_mode(&m_input, LineInputModeFunctions, is_enable);
    }

    /**
     * 行単位入力モードかどうかを得る
     * 
     * @retval true 行単位入力モード
     * @retval false 文字単位入力モード
     */
    bool is_line_input_mode(void) const noexcept {
        return ((m_input.mode & LineInputModeFunctions) == LineInputModeFunctions) ? true : false;
    }

    /**
     * 読み出しバッファのサイズを設定する。
     * 
     * @param length 長さ
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
     * 読み出しバッファのサイズを取得する。
     * 
     * @retval バッファサイズ
     */
    uint32_t get_max_read_length(void) const noexcept {
        return m_max_read_length;
    }

    /**
     * 読み出しバッファにたまっているデータ量を取得する。
     * 
     * @retval データ量
     */
    uint32_t get_read_data_length(void) const noexcept {
        return static_cast<uint32_t>(m_input_data.size());
    }

    /**
     * 入力が終端したかどうかを判定する。
     * 
     * @retval true 終端している場合
     * @retval false 終端していない場合
     */
    bool is_input_EOF(void) const noexcept { return Terminated; }
    /**
     * 論理型へのキャスト演算子
     * 
     * @retval true 入力が有効な場合
     * @retval false 入力が無効な場合
     */
    operator bool() const noexcept {
        return is_input_valid() && !is_input_EOF();
    }
    /**
     * 否定演算子
     * 
     * @retval true 入力が無効な場合
     * @retval false 入力が有効な場合
     */
    bool operator!() const noexcept {
        return !this->operator bool();
    }

    /**
     * 入力が有効かどうか。
     * コンソールに接続されているか、リダイレクトされている場合にtrue。
     * どっかの子プロセスとして起動されて、リダイレクトされていない場合にはfalse.
     * 
     * @retval true 有効
     * @retval false 無効
     */
    bool is_input_valid(void) const noexcept {
        return (m_input.handle != INVALID_HANDLE_VALUE) ? true : false;
    }
    /**
     * 標準出力が有効かどうか。
     * コンソールに接続されているか、リダイレクトされている場合にtrue。
     * どっかの子プロセスとして起動されて、リダイレクトされていない場合にはfalse.
     *
     * @retval true 有効
     * @retval false 無効
     */
    bool is_output_valid(void) const noexcept {
        return (m_output.handle != INVALID_HANDLE_VALUE) ? true : false;
    }
    /**
     * 標準エラー出力が有効かどうか。
     * コンソールに接続されているか、リダイレクトされている場合にtrue。
     * どっかの子プロセスとして起動されて、リダイレクトされていない場合にはfalse.
     *
     * @retval true 有効
     * @retval false 無効
     */
    bool is_error_valid(void) const noexcept {
        return (m_error.handle != INVALID_HANDLE_VALUE) ? true : false;
    }

    /**
     * 1行読み出す。
     * 入力が終端している場合には空文字列が返る。
     * 改行コードを検出する前に入力が終端した場合には、途中までの文字列が返る。
     * 返す文字列は改行コードを含む。
     * 
     * @retval 行
     */
    std::string read_line(void);
    /**
     * 改行コードを検出するか、bufsize - 1文字読むまで読み出す。
     * 
     * @param buf バッファ(少なくとも2バイト以上格納できるような領域とすること)
     * @param bufsize バッファサイズ
     * @param plength 読み出した長さを格納する変数(不要な場合にはnullptr)
     * @retval true 成功
     * @retval false 失敗(不正なパラメータが渡された場合)
     */
    bool read_line(char* buf, size_t bufsize, size_t* plength = nullptr);
    /**
     * 入力バッファから最大でbufsizeだけ読み出す。
     * timeoutで指定した時間だけ待機する。
     * 
     * @param buf バッファ
     * @param bufsize バッファサイズ
     * @param pread 読み出した長さを格納する変数
     * @param timeout タイムアウト時間[ミリ秒](負数にすると永遠に待つ)
     * @retval true 成功
     * @retval false 失敗
     */
    bool read_with_timeout(void* buf, size_t bufsize, size_t* pread, int32_t timeout);
    /**
     * 入力バッファから最大でbufsizeだけ読み出す。
     * 本インタフェースは入力待ちをしない。
     * 
     * @param buf バッファ
     * @param bufsize バッファサイズ
     * @param pread 読み出した長さを格納する変数
     * @retval true 成功
     * @retval false 失敗
     */
    bool read(void* buf, size_t bufsize, size_t* pread);

    /**
     * 文字列を書式指定で出力する。
     * 
     * @param fmt フォーマット
     * @param args パラメータ
     * @retval true 成功
     * @retval false 失敗
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
     * 文字列を出力する。
     * 
     * @param str 文字列
     * @retval true 成功
     * @retval false 失敗
     */
    bool print(const std::string& str) {
        return print(&m_output, str);
    }

    /**
     * 文字を出力する。
     * 
     * @param c 文字
     * @retval true 成功
     * @retval false 失敗
     */
    bool print(char c) {
        char buf[1];
        buf[0] = c;
        return write(&m_output, buf, 1);
    }
    /**
     * 文字列を書式指定で標準エラー出力に出力する。
     *
     * @param fmt フォーマット
     * @param args パラメータ
     * @retval true 成功
     * @retval false 失敗
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
     * 文字列を標準エラー出力に出力する。
     *
     * @param str 文字列
     * @retval true 成功
     * @retval false 失敗
     */
    bool print_err(const std::string& str) {
        return print(&m_error, str);
    }
    /**
     * 文字を標準エラー出力に出力する。
     *
     * @param c 文字
     * @retval true 成功
     * @retval false 失敗
     */
    bool print_err(char c) {
        char buf[1];
        buf[0] = c;
        return write(&m_error, buf, 1);
    }

    /**
     * データを出力する。
     * 
     * @param data データのアドレス
     * @param length データの長さ
     * @param pwritten 出力したデータ長を受け取る変数(不要な場合はnullptr)
     * @retval true 成功
     * @retval false 失敗
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
     * データを標準エラー出力に出力する。
     * 
     * @param data データのアドレス
     * @param length データの長さ
     * @param pwritten 出力したデータ長を受け取る変数(不要な場合はnullptr)
     * @retval true 成功
     * @retval false 失敗
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
        HANDLE handle; // ハンドル
        bool is_console; // コンソールかどうか
        DWORD mode; // コンソールモード
        std_io(void) : handle(INVALID_HANDLE_VALUE), is_console(false), mode(0) { }
    };
    bool m_initialized; // 初期化したかどうかのフラグ
    std_io m_input; // 標準入力
    std_io m_output; // 標準出力
    std_io m_error; // 標準エラー出力
    std::queue<uint8_t> m_input_data; // 入力データ
    std::mutex m_input_lock; // 入力ロック
    uint32_t m_max_read_length; // 読み出しバッファサイズ
    char m_prev_input_data; // 前回入力文字
    static bool Terminated; // 終端検知したか
    static const DWORD LineInputModeFunctions; // 行単位入力モード機能

    /**
     * コンストラクタ
     */
    StandardIo(void);
    /**
     * 初期化する。
     * 
     * @retval true 成功
     * @retval false 失敗
     */
    bool init(void);
    /**
     * 初期化したかどうか
     * 
     * @retval true 初期化済み
     * @retval false 未初期化
     */
    bool is_initialized(void) const noexcept { return m_initialized; }

    /**
     * ハンドルを初期化する。
     * 
     * @param pstdio I/Oオブジェクト
     * @param handle_type ハンドルタイプ
     */
    void init_std_handle(std_io* pstdio, DWORD handle_type);
    /**
     * コンソールモードを設定する。
     * 
     * @param pstdio I/Oオブジェクト
     * @param functions 対象の機能
     * @param is_enabled 有効にする場合はtrue, 無効にする場合はfalse.
     */
    bool modify_console_mode(std_io* pstdio, DWORD functions, bool is_enabled);
    /**
     * 出力する
     * 
     * @param pstdio I/Oオブジェクト
     * @param str 出力文字列
     * @retval true 成功
     * @retval false 失敗
     */
    bool print(std_io* pstdio, const std::string &str);
    /**
     * 出力する
     *
     * @param pstdio I/Oオブジェクト
     * @param data 送信データポインタ
     * @param length 長さ
     * @param pwritten 書き込んだ長さ(不要な場合にはnullptrを渡す)
     * @retval true 成功
     * @retval false 失敗
     */
    bool write(std_io* pstdio, const void* data, size_t length, size_t* pwritten = nullptr);

    /**
     * 改行コードをCR+LFに揃える
     * 
     * @param str 文字列
     * @retval 置換した文字列
     */
    std::string replace_CRLF(const std::string& str);

    /**
     * 受信スレッド処理
     */
    void receiver_thread_proc(void);
    /**
     * コンソールからの入力読み出しを行う。
     */
    void read_from_console(void);
    /**
     * パイプからの入力読み出しを行う。
     */
    void read_from_pipe(void);

    /**
     * コンソールハンドラ
     * 
     * @param event イベント
     */
    static BOOL console_handler_proc(DWORD event);

    StandardIo(const StandardIo& io) = delete;
    StandardIo& operator=(const StandardIo& io) = delete;
};

