#pragma once
#include <system_error>

constexpr int APP_ERROR_SUCCESS = 0;
constexpr int APP_ERROR_NO_SERIAL_PORTS = 1;

/**
 * アプリケーションのエラーカテゴリを得る。
 *
 * @retval エラーカテゴリオブジェクト
 */
const std::error_category& app_error_category(void);
