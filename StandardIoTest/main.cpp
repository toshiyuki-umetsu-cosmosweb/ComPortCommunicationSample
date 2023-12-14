// ConsoleTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <Windows.h>
#include <cstdarg>
#include <cstdint>
#include <csignal>
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <sstream>
#include <StandardIo.h>
#include <utils.h>

static bool proc_cmdline(const std::string& line);
static bool IsAppRun = true;

int main(int ac, char** av)
{
    auto& stdio = StandardIo::instance();

    {
        SetConsoleCtrlHandler([](DWORD event) {
            BOOL retval = FALSE;
            if (event == CTRL_C_EVENT) {
                IsAppRun = false;
                retval = TRUE;
            }
            return retval;
            }, TRUE);
    }
    stdio.print_err("started.");

    // Ctrl-Break による中断 -> OK
    // 行読み込み -> OK
    // 行単位入力 -> OK
    // 文字単位入力 -> OK

    std::ostringstream line_buf;

    stdio.set_line_input_mode(true);
    while (IsAppRun) {
        if (stdio.is_line_input_mode()) {
            stdio.print("> ");
            const std::string& line = stdio.read_line();
            if ((line.length() == 0) && stdio.is_input_EOF()) {
                IsAppRun = false;
            }
            else {
                if (proc_cmdline(line)) {
                    // do nothing.
                }
                else {
                    stdio.print(line);
                }
            }
            line_buf.str("");
        }
        else {
            char buf[256];
            size_t read_len;
            if (stdio.read(buf, sizeof(buf), &read_len)) {
                if ((read_len == 0) && stdio.is_input_EOF()) {
                    IsAppRun = false;
                }
                else {
                    for (size_t i = 0; i < read_len; i++) {
                        char c = static_cast<char>(buf[i]);
                        stdio.print(buf[i]);
                        line_buf << c;
                        if (c == '\n') {
                            auto line = line_buf.str();
                            proc_cmdline(line);
                            line_buf.str("");
                        }
                    }
                }
            }
        }

    }
    stdio.print_err("Exit.\n");

	return 0;
}

static bool proc_cmdline(const std::string& line) {
    bool is_processed = false;

    auto& stdio = StandardIo::instance();
    arg_t args;
    make_argv(line, &args);
    if (args.size() > 0) {
        if (args[0] == "q") {
            IsAppRun = false;
            is_processed = true;
        }
        else if (args[0] == "line") {
            stdio.set_line_input_mode(true);
            stdio.print("Set line input mode.\n");
            is_processed = true;
        }
        else if (args[0] == "char") {
            stdio.set_line_input_mode(false);
            stdio.print("Set character input mode.\n");
            is_processed = true;
        }
        else {
            // do nothing.
        }
    }
    else {
        // do nothing.
    }
    return is_processed;
}


