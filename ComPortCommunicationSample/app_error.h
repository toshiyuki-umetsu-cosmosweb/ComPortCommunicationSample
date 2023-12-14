#pragma once
#include <system_error>

constexpr int APP_ERROR_SUCCESS = 0;
constexpr int APP_ERROR_NO_SERIAL_PORTS = 1;

/**
 * �A�v���P�[�V�����̃G���[�J�e�S���𓾂�B
 *
 * @retval �G���[�J�e�S���I�u�W�F�N�g
 */
const std::error_category& app_error_category(void);
