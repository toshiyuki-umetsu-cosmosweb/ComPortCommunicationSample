#include <Windows.h>
#include "utils.h"
#include "WindowsErrorCategory.h"



/**
 *  Windows�̃G���[���`����N���X�B
 */
class WindowsErrorCategory : public std::error_category
{
public:
    /**
     * �J�e�S�������擾����
     *
     * @retval �J�e�S����
     */
    const char* name(void) const noexcept override {
        return "Windows system error";
    }

    /**
     * �G���[���b�Z�[�W���擾����B
     *
     * @param ev �G���[�ԍ�
     * @retval �G���[���b�Z�[�W
     */
    std::string message(int ev) const override {
        return get_windows_error_message(ev);
    }
};

const std::error_category& windows_error_category(void) {
    static WindowsErrorCategory category;
    return category;
}

