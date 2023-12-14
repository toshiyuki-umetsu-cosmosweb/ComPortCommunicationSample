#include "app_error.h"

class AppErrorCategory : public std::error_category {
public:
    /**
     * カテゴリ名を取得する
     *
     * @retval カテゴリ名
     */
    const char* name(void) const noexcept override {
        return "Application error";
    }

    /**
     * エラーメッセージを取得する。
     *
     * @param ev エラー番号
     * @retval エラーメッセージ
     */
    std::string message(int ev) const override {
        const char* msg;
        switch (ev) {
        case APP_ERROR_SUCCESS:
            msg = "No errors.";
            break;
        case APP_ERROR_NO_SERIAL_PORTS:
            msg = "No serial ports exists.";
            break;
        default:
            msg = "Unknown error.";
            break;
        }
        return std::string(msg);
    }
};


const std::error_category& app_error_category(void) {
    static AppErrorCategory category;
    return category;
}