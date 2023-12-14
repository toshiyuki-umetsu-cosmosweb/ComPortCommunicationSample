#pragma once

#include <string>
#include <system_error>



/**
 * Windowsのエラーカテゴリを得る。
 *
 * @retval エラーカテゴリオブジェクト
 */
const std::error_category& windows_error_category(void);



