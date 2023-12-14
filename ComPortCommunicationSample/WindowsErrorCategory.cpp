#include <Windows.h>
#include "utils.h"
#include "WindowsErrorCategory.h"



/**
 *  Windowsのエラーを定義するクラス。
 */
class WindowsErrorCategory : public std::error_category
{
public:
    /**
     * カテゴリ名を取得する
     *
     * @retval カテゴリ名
     */
    const char* name(void) const noexcept override {
        return "Windows system error";
    }

    /**
     * エラーメッセージを取得する。
     *
     * @param ev エラー番号
     * @retval エラーメッセージ
     */
    std::string message(int ev) const override {
        return get_windows_error_message(ev);
    }
};

const std::error_category& windows_error_category(void) {
    static WindowsErrorCategory category;
    return category;
}

