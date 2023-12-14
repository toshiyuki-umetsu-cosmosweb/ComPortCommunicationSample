// ConsoleTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <Windows.h>
#include <iostream>

int main(void)
{
    static bool IsAppRun = true; // アプリケーション動作フラグ。 falseにするとループを抜ける
    {
        // Setup ConsoleMode to disable line input.
        auto h = GetStdHandle(STD_INPUT_HANDLE);
        if ((h != INVALID_HANDLE_VALUE) && (h != nullptr)) {
            DWORD mode;
            if (GetConsoleMode(h, &mode)) {
                mode &= ~(ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_QUICK_EDIT_MODE);
                if (SetConsoleMode(h, mode)) {
                    // do nothing.
                }
                else {
                    std::cerr << "SetConsoleMode() failure." << std::endl;
                }
            }
            else {
                std::cerr << "GetConsoleMode() failure." << std::endl;
            }
        }

        // Ctrl-C, Ctrl-Breakを受け付けて処理するイベントハンドラを登録
        SetConsoleCtrlHandler([](DWORD evt) { 
                if ((evt == CTRL_C_EVENT)
                    || (evt == CTRL_BREAK_EVENT)) {
                    IsAppRun = false;
                    return TRUE;
                }
                else {
                    return FALSE;
                }
            }, TRUE);
    }

    while (IsAppRun) {
        char buf[2];
        std::cin.read(buf, sizeof(buf));
        std::cout << buf[0];
    }


    return 0;
}
