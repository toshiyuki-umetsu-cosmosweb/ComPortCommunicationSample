#include "app_error.h"

class AppErrorCategory : public std::error_category {
public:
    /**
     * �J�e�S�������擾����
     *
     * @retval �J�e�S����
     */
    const char* name(void) const noexcept override {
        return "Application error";
    }

    /**
     * �G���[���b�Z�[�W���擾����B
     *
     * @param ev �G���[�ԍ�
     * @retval �G���[���b�Z�[�W
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