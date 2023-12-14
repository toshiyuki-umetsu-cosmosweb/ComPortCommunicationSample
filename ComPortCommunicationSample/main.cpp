// SRM0274Test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <tchar.h>
#include <Windows.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <list>
#include <thread>
#include <vector>
#include <system_error>
#include <stdexcept>

#include "utils.h"
#include "StandardIo.h"
#include "WindowsErrorCategory.h"
#include "SerialPort.h"
#include "app_error.h"


struct CommandEntry {
    const char* command; // コマンド文字列
    const char* description; // ディスクリプション
    void (*cmd_proc)(arg_t& arg); // 処理

    CommandEntry(const char* cmd, const char* desc, void (*proc)(arg_t& arg))
        : command(cmd), description(desc), cmd_proc(proc) {

    }
};

/**
 * コマンドエントリリスト
 */
static std::vector<CommandEntry> CommandEntries;

/**
 * シリアルポートインスタンス
 */
static std::unique_ptr<SerialPort> SerialPortPtr;

/**
 * アプリケーション実行フラグ。
 */
static bool IsAppRun;

enum ApplicationMode {
    AppModeSetup,
    AppModeCommunication
};

/**
 * アプリケーション動作モード
 */
static enum ApplicationMode ApplicationMode = AppModeCommunication;

static const StringValueList ParityValueEntries = {
    { "none", SerialPort::ParityNone },
    { "even", SerialPort::ParityEven },
    { "odd", SerialPort::ParityOdd }
};



struct ApplicationSetting {
    uint32_t baudrate; // ボーレート
    uint32_t parity; // パリティ
    uint32_t stopbits; // ストップビット
    uint8_t databits; // データビット数
    uint32_t cts_flow; // CTS フロー制御
    uint32_t rts_control; // RTS制御
    std::string port_name; // オープンポート名(空文字列で指定無し)
    ApplicationSetting()
        : baudrate(115200), parity(SerialPort::ParityNone), stopbits(SerialPort::StopBitsOne),
        databits(8), cts_flow(SerialPort::CtsFlowDisable), rts_control(SerialPort::RtsControlEnable),
        port_name("") {
    }
};

struct CommandLineOption {
    const char* option; // オプション文字列("--”を抜いたやつ)
    const char* help; // ヘルプメッセージ
    int arg_count; // 引数の数
    void (*parser)(ApplicationSetting* psetting, arg_t& arg); // パーサー
    CommandLineOption(const char *option, const char *help, int arg_count, void (*parser)(ApplicationSetting*, arg_t&))
        : option(option), help(help), arg_count(arg_count), parser(parser) {
    }
};


static BOOL on_console_event(DWORD event);

static const std::vector<CommandLineOption>& get_command_line_options(void);
static void parse_option_baudrate(ApplicationSetting* psetting, arg_t& opt_args);
static void parse_option_parity(ApplicationSetting* psetting, arg_t& opt_args);
static void parse_option_stopbits(ApplicationSetting* psetting, arg_t& opt_args);
static void parse_option_cts_flow(ApplicationSetting* psetting, arg_t& opt_args);
static void parse_option_rts_control(ApplicationSetting* psetting, arg_t& opt_args);
static void print_usage(void);
static void proc_args(ApplicationSetting* psetting, int ac, char** av);

static void update_command_list(void);
static bool select_serial_port_proc(const std::vector<std::string>& port_list, int* pselected);
static void command_proc(arg_t& args);
static void receiver_thread_proc(void);
static void cmd_argv(arg_t& args);
static void cmd_help(arg_t& args);
static void cmd_quit(arg_t& args);
static void cmd_open(arg_t& args);
static void cmd_baudrate(arg_t& args);
static void cmd_parity(arg_t& args);

/**
 * アプリケーションのエントリポイント
 * 
 * @param ac 引数
 * @param argv 引数配列
 * @retval 0 正常終了
 * @retval 0以外 異常終了
 */
int main(int ac, char** argv)
{
    auto& stdio = StandardIo::instance();
    try {
        ApplicationSetting setting;

        if ((ac == 2) && (strcmp(argv[1], "--help") == 0)) {
            print_usage();
            return EXIT_SUCCESS;
        }
        else {
            proc_args(&setting, ac - 1, argv + 1);
        }

        std::vector<std::string> port_list;
        SerialPort::enumerate_ports(&port_list);
#ifdef _DEBUG
        for (const std::string& s : port_list) {
            stdio.print_err("Found COM port:%s\n", s.c_str());
        }
#endif
        std::string selected_serial_port = setting.port_name;
        if (selected_serial_port.empty()) { // シリアルポート指定無し？
            if (port_list.empty()) {
                throw std::system_error(APP_ERROR_NO_SERIAL_PORTS, app_error_category());
            }
            else if (port_list.size() > 1) { // シリアルポートが複数ある？
                int selected_index;
                if (select_serial_port_proc(port_list, &selected_index)) {
                    return EXIT_FAILURE;
                }

                selected_serial_port = port_list[selected_index];
            }
            else {
                selected_serial_port = port_list[0];
            }
        }
        stdio.print("Selected serial port: %s\n", selected_serial_port.c_str());

        SetConsoleCtrlHandler(on_console_event, TRUE);

        update_command_list();

        SerialPortPtr = std::make_unique<SerialPort>(selected_serial_port);

        (*SerialPortPtr).set_baudrate(setting.baudrate);
        (*SerialPortPtr).set_parity(setting.parity);
        (*SerialPortPtr).set_stopbits(setting.stopbits);
        (*SerialPortPtr).set_databits(setting.databits);
        (*SerialPortPtr).set_cts_flow(setting.cts_flow);
        (*SerialPortPtr).set_rts_control(setting.rts_control);
        try {
            (*SerialPortPtr).open();
            ApplicationMode = AppModeCommunication;
            stdio.set_line_input_mode(false);
        }
        catch (std::exception& e) {
            stdio.print_err(e.what());
            ApplicationMode = AppModeSetup;
            stdio.set_line_input_mode(true);
        }

        std::thread thread(receiver_thread_proc);
        stdio.print_err("Press Ctrl-C to change setting mode.\n");

        IsAppRun = true;
        while (IsAppRun) {
            if (ApplicationMode == AppModeSetup) {
                stdio.print("> ");

                std::string line = stdio.read_line();
                if (line.empty() && !stdio) { // 入力終端した？
                    break;
                }
                arg_t argv;
                make_argv(line, &argv);
                if (argv.size() > 0) {
                    command_proc(argv);
                }
            }
            else {
                uint8_t buf[256];
                size_t read_len;
                if (stdio.read(buf, sizeof(buf), &read_len)) {
                    if (read_len > 0) {
                        (*SerialPortPtr).send(buf, static_cast<uint32_t>(read_len));
                    }
                    else if (!stdio) {
                        // Note: 受信しっぱなしを許容するため、
                        //       入力がBREAKしても終了しない。
                    }
                    else {
                        // do nothing.
                    }
                }
            }
        }
        IsAppRun = false;
        (*SerialPortPtr).close();
        thread.join();
    }
    catch (std::exception& ex) {
        stdio.print_err("%s\n", ex.what());
    }
    if (SerialPortPtr != nullptr) {
        SerialPortPtr.reset();
    }

    return EXIT_SUCCESS;
}

/**
 * コンソール端末でイベントが発生したときに通知を受け取る。
 * 
 * @param event イベント
 * @retval TRUE イベントを処理した場合
 * @retval FALSE イベントを処理しない場合
 */
static BOOL on_console_event(DWORD event) {
    BOOL retval;
    switch (event) {
    case CTRL_C_EVENT:
    {
        if (ApplicationMode == AppModeCommunication) {
            (*SerialPortPtr).close();
            ApplicationMode = AppModeSetup;
            StandardIo::instance().set_line_input_mode(true);
            update_command_list();
        }
        else {
            IsAppRun = false;
        }
        retval = TRUE;
        break;
    }
    default:
        retval = FALSE;
        break;
    }
    return retval;
}


/**
 * コマンドラインオプション配列を得る。
 * 
 * @param コマンドラインオプション配列
 */
static const std::vector<CommandLineOption>& get_command_line_options(void) {
    static std::vector<CommandLineOption> options;

    if (options.empty()) {
        options.push_back(CommandLineOption("-baudrate", "Specify baudrate[bps].", 1, parse_option_baudrate));
        options.push_back(CommandLineOption("-parity", "Specify parity. ('none','even','odd')", 1, parse_option_parity));
        options.push_back(CommandLineOption("-stopbits", "Specify stopbits. ('1', '1.5', '2')", 1, parse_option_stopbits));
        options.push_back(CommandLineOption("-cts-flow", "Specify CTS flow control. ('enable','disable')", 1, parse_option_cts_flow));
        options.push_back(CommandLineOption("-rts-control", "Specify RTS control. ('low','high','handshake','toggle')", 1, parse_option_rts_control));
    }

    return options;
}

/**
 * --baudrate オプションを解析して設定する。
 * 
 * @param psetting 設定
 * @param opt_args オプションの引数
 */
static void parse_option_baudrate(ApplicationSetting* psetting, arg_t& opt_args) {
    uint32_t baudrate;
    if (parse_ui32(opt_args[0], &baudrate)) {
        (*psetting).baudrate = baudrate;
    }
    else {
        throw std::invalid_argument(format("Invalid baudrate : %s", opt_args[0].c_str()));
    }
}

/**
 * --parity オプションを解析して設定する。
 *
 * @param psetting 設定
 * @param opt_args オプションの引数
 */
static void parse_option_parity(ApplicationSetting* psetting, arg_t& opt_args) {
    uint32_t parity;
    if (parse_value(ParityValueEntries, opt_args[0], &parity)) {
        (*psetting).parity = parity;
    }
    else {
        throw std::invalid_argument(format("Invalid parity : %s", opt_args[0]));
    }

}

/**
 * --stopbits オプションを解析して設定する。
 *
 * @param psetting 設定
 * @param opt_args オプションの引数
 */
static void parse_option_stopbits(ApplicationSetting* psetting, arg_t& opt_args) {
    const StringValueList Entries = {
        { "1", SerialPort::StopBitsOne },
        { "1.5", SerialPort::StopBitsOne5 },
        { "2", SerialPort::StopBitsTwo }
    };
    uint32_t stopbits;
    if (parse_value(Entries, opt_args[0], &stopbits)) {
        (*psetting).stopbits = stopbits;
    }
    else {
        throw std::invalid_argument(format("Invalid parity : %s", opt_args[0]));
    }
}

/**
 * --cts_flow オプションを解析して設定する。
 *
 * @param psetting 設定
 * @param opt_args オプションの引数
 */
static void parse_option_cts_flow(ApplicationSetting* psetting, arg_t& opt_args) {
    const StringValueList Entries = {
        { "enable", SerialPort::CtsFlowEnable },
        { "disable", SerialPort::CtsFlowDisable },
    };
    uint32_t cts_flow;
    if (parse_value(Entries, opt_args[0], &cts_flow)) {
        (*psetting).cts_flow = cts_flow;
    }
    else {
        throw std::invalid_argument(format("Invalid parity : %s", opt_args[0]));
    }
}

/**
 * --rts_control オプションを解析して設定する。
 *
 * @param psetting 設定
 * @param opt_args オプションの引数
 */
static void parse_option_rts_control(ApplicationSetting* psetting, arg_t& opt_args) {
    const StringValueList Entries = {
        { "low", SerialPort::RtsControlDisable },
        { "high", SerialPort::RtsControlEnable },
        { "handshake", SerialPort::RtsControlHandShake },
        { "toggle", SerialPort::RtsControlToggle }
    };
    uint32_t rts_control;
    if (parse_value(Entries, opt_args[0], &rts_control)) {
        (*psetting).rts_control = rts_control;
    }
    else {
        throw std::invalid_argument(format("Invalid parity : %s", opt_args[0]));
    }
}

/**
 * アプリケーションの使用方法を表示する。
 */
static void print_usage(void) {
    auto& stdio = StandardIo::instance();
    auto pname = get_process_filename();

    auto options = get_command_line_options();
    stdio.print("Usage:\n");
    stdio.print("  %s [options] [port_name$] - Run application.\n", pname.c_str());
    stdio.print("  %s -help - Print this message.\n", pname.c_str());
    stdio.print("Options:\n");
    for (auto& option : options) {
        stdio.print("  -%s : %s\n", option.option, option.help);
    }
    return;
}

/**
 * アプリケーション引数のコマンドを解析処理する。
 * 
 * @param psetting 設定
 * @param ac 引数の数
 * @param av 引数配列
 */
static void proc_args(ApplicationSetting* psetting, int ac, char** av) {
    auto options = get_command_line_options();

    for (int i = 0; i < ac; i++) {
        if (av[i][0] == '-') { // "-"で始まってる？
            const char* opt = &(av[i][1]);
            auto pentry = std::find_if(options.begin(), options.end(),
                [opt](const CommandLineOption& option_entry) { return strcmp(opt, option_entry.option) == 0; });
            if (pentry == options.end()) {
                throw std::invalid_argument(format("Unknown option : %s", av[i]));
            }
            if ((i + (*pentry).arg_count) >= ac) {
                throw std::invalid_argument(format("Too few arguments for '-%s' option.", (*pentry).option));
            }

            arg_t opt_args;
            for (int j = 0; j < (*pentry).arg_count; j++) {
                opt_args.push_back(std::string(av[i + j + 1]));
            }
            (*pentry).parser(psetting, opt_args);
            i += (*pentry).arg_count;
        }
        else {
            if ((i + 0) < ac) {
                (*psetting).port_name = std::string(av[i + 0]);
            }
            break;
        }
        i++;
    }

    return;
}

/**
 * 現在の動作モードにあわせてコマンドリストを更新する。
 */
static void update_command_list(void) {
    CommandEntries.clear();
    CommandEntries.push_back(CommandEntry("open", "Open serial I/O mode.", cmd_open));
    CommandEntries.push_back(CommandEntry("baudrate", "Set/Get baudrate.", cmd_baudrate));
    CommandEntries.push_back(CommandEntry("parity", "Set/Get parity", cmd_parity));
    CommandEntries.push_back(CommandEntry("argv", "Print argv.", cmd_argv));
    CommandEntries.push_back(CommandEntry("help", "Print help messages.", cmd_help));
    CommandEntries.push_back(CommandEntry("quit", "Quit application.", cmd_quit));
    CommandEntries.push_back(CommandEntry("q", "", cmd_quit));

    return;
}

/**
 * シリアルポート選択を行う。
 * 
 * @param port_list ポートリスト
 * @param pselected port_list上の選択されたインデックス番号
 * @retval true 選択操作が完了した場合
 * @retval false キャンセルされた場合
 */
static bool select_serial_port_proc(const std::vector<std::string>& port_list, int* pselected)
{
    auto& stdio = StandardIo::instance();

    if (!stdio.is_input_valid()) {
        stdio.print_err("Port selection not work.\n");
        return false;
    }

    stdio.set_line_input_mode(true);
    while (true) {
        stdio.print("Select serial port.\n");
        for (int i = 0; i < port_list.size(); i++) {
            stdio.print("  [%d]:%s\n", i, port_list[i].c_str());
        }
        stdio.print("  Enter number > ");

        std::string line = stdio.read_line();
        if (line.empty() && !stdio) { // 入力終端した？
            return false;
        }
        arg_t args;
        make_argv(line, &args);

        if (args.size() > 0) {
            uint32_t no;
            if (parse_ui32(args[0], &no)) {
                if ((no >= 0) && (no < port_list.size())) {
                    (*pselected) = no;
                    return true;
                }
                else {
                    stdio.print_err("Index must be 0 <= no < %d\n",
                        static_cast<int>(port_list.size()));
                }
            }
            else {
                stdio.print_err("Invalid number. [%s]\n", args[0].c_str());
            }
        }
    }

}

/**
 * コマンドを処理する
 * 
 * @param args コマンドライン引数配列
 */
static void command_proc(arg_t &args) {
    const std::string& cmd = args[0];

    auto it = std::find_if(CommandEntries.begin(), CommandEntries.end(), 
        [ cmd ](CommandEntry& entry) { return cmd == entry.command; });
    if (it == CommandEntries.end()) {
        StandardIo::instance().print_err("Unknown command. [%s]\n", cmd.c_str());
        return;
    }

    (*it).cmd_proc(args);

    return;
}

/**
 * 受信スレッド処理
 */
static void receiver_thread_proc(void) {
    auto& stdio = StandardIo::instance();

    uint8_t recv_buf[256];

    while (IsAppRun) {
        if ((*SerialPortPtr).is_opened()) {
            int result = (*SerialPortPtr).receive(recv_buf, sizeof(recv_buf), 100);
            if (result > 0) {
                stdio.write(recv_buf, result);
            }
            else {
                std::this_thread::yield(); // 別スレッドにスイッチ。
            }
        }
        else {
            std::this_thread::yield(); // 別スレッドにスイッチ
        }
    }

    return ;
}




/**
 * argv コマンドを処理する。
 *
 * @param args 引数
 */
static void cmd_argv(arg_t& args) {
    auto& stdio = StandardIo::instance();
    for (int i = 0; i < args.size(); i++) {
        stdio.print("argv[%d]:%s\n", i, args[i].c_str());
    }
    return;
}


/**
 * help コマンドを処理する。
 *
 * @param args 引数
 */
static void cmd_help(arg_t& args) {
    auto& stdio = StandardIo::instance();
    for (CommandEntry& entry : CommandEntries) {
        if (strlen(entry.description) > 0) {
            stdio.print("%s - %s\n", entry.command, entry.description);
        }
    }
    return;
}
/**
 * quit コマンドを処理する。
 *
 * @param args 引数
 */
static void cmd_quit(arg_t& args) {
    IsAppRun = false;

    return;
}

/**
 * open コマンドを処理する。
 * 
 * @param args 引数
 */
static void cmd_open(arg_t& args) {
    auto& stdio = StandardIo::instance();
    try {
        if (args.size() >= 2) {
            std::string& port_name = args[1];
            SerialPortPtr = std::make_unique<SerialPort>(port_name, (*SerialPortPtr));
        }

        (*SerialPortPtr).open();
        ApplicationMode = AppModeCommunication;
        stdio.set_line_input_mode(false);
    }
    catch (std::exception e) {
        stdio.print_err(e.what());
    }
}

/**
 * baudrate コマンドを処理する。
 *
 * @param args 引数
 */
static void cmd_baudrate(arg_t& args) {
    auto& stdio = StandardIo::instance();
    if (args.size() >= 2) {
        uint32_t baudrate;
        if (parse_ui32(args[1], &baudrate)) {
            (*SerialPortPtr).set_baudrate(baudrate);
        }
        else {
            stdio.print_err("Invalid baudrate %s\n", args[1]);
        }
    }
    else {
        stdio.print("%u\n", (*SerialPortPtr).get_baudrate());
    }

    return;
}

/**
 * parity コマンドを処理する。
 *
 * @param args 引数
 */
static void cmd_parity(arg_t& args) {
    auto& stdio = StandardIo::instance();
    if (args.size() >= 2) {
        uint32_t parity;
        if (parse_value(ParityValueEntries, args[1], &parity)) {
            (*SerialPortPtr).set_parity(parity);
        }
        else {
            stdio.print_err("Invalid parity value. %s\n", args[1]);
        }
    }
    else {
        auto it = find_value(ParityValueEntries, (*SerialPortPtr).get_parity());
        if (it != ParityValueEntries.end()) {
            stdio.print("%s\n", (*it).name);
        }
    }
}
