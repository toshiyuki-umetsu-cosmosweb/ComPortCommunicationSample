#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

/**
 * str����͂��ĕ�������32bit�����𓾂�B
 * 
 * @param str ������
 * @param pvalue ���������������i�[����ϐ�
 * @retval true ����
 * @retval false ���s
 */
bool parse_ui32(const std::string& str, uint32_t* pvalue);
/**
 * str����͂��ĕ�������32bit�����𓾂�B
 *
 * @param str ������
 * @param pvalue ���������������i�[����ϐ�
 * @retval true ����
 * @retval false ���s
 */
bool parse_ui32(const char* str, uint32_t* pvalue);
/**
 * str����͂��ĕ�������32bit�����𓾂�B
 *
 * @param str ������
 * @param pvalue �������萮�����i�[����ϐ�
 * @retval true ����
 * @retval false ���s
 */
bool parse_i32(const std::string& str, int32_t* pvalue);
/**
 * str����͂��ĕ�������32bit�����𓾂�B
 *
 * @param str ������
 * @param pvalue �������萮�����i�[����ϐ�
 * @retval true ����
 * @retval false ���s
 */
bool parse_i32(const char* str, int32_t* pvalue);

/**
 *  str�̐擪�Ɩ����ɂ���delim�Ɋ܂܂�镶���������ĕԂ��B
 *
 * @param str ������
 * @param delim �f���~�^
 * @retval delim�������ꂽ������B
 */
std::string strtrim(const std::string& str, const std::string &delim = std::string(" \t\r\n"));
/**
 *  str�̐擪�Ɩ����ɂ���delim�Ɋ܂܂�镶���������ĕԂ��B
 *
 * @param str ������
 * @param delim �f���~�^
 * @retval delim�������ꂽ������B
 */
char* strtrim(char* str, const char* delim = " \t\r\n");

typedef std::vector<char*> argchar_t;
/**
 * str���g�[�N����������B
 *
 * @param str ������
 * @param pargv ���������g�[�N�����i�[���郊�X�g
 * @param delim �f���~�^
 */
void make_argv(char* str, argchar_t* pargv, const char *delim = " \t\r\n");

typedef std::vector<std::string> arg_t;
/**
 * str���g�[�N����������B
 *
 * @param str ������
 * @param pargv ���������g�[�N�����i�[����z��
 * @param delim �����Ɏg�p�����؂蕶���̏W��
 */
void make_argv(const std::string& str, arg_t* pargv, const std::string &delim = std::string(" \t\r\n"));

/**
 * �v���Z�X�̃t�@�C�����𓾂�B
 *
 * Note: �[���f�B���N�g��(�p�X������MAX_PATH�𒴂���P�[�X)�ł́A���������삵�Ȃ��B
 *
 * @retval �v���Z�X�̃t�@�C�����B
 */
std::string get_process_filename(void);


/**
 * �����w�肵�ĕ�����𐶐�����B
 * C++11�p�B
 * 
 * https://pyopyopyo.hatenablog.com/entry/2019/02/08/102456 ���Q�l�ɂ����Ă����������B
 * Note: C++11�p�B C++20 �ł�std::format()������̂ł�������g���ׂ��B
 */
template <typename ... Args>
std::string format(const char* fmt, Args ... args)
{
    if (fmt != nullptr) {
        size_t len = snprintf(nullptr, 0, fmt, args ...);
        std::vector<char> buf(len + 1);
        snprintf(&buf[0], len + 1, fmt, args ...);
        return std::string(&buf[0], &buf[0] + len);
    }
    else {
        return std::string("");
    }
}

/**
 * ������ �� 32bit������������ ���y�A�ɂ����l�G���g���B
 * �O��(�g�p��)�Ƃ�I/F�͕�����Ƃ��āA�v���O���������ł͐��l�ň������߂̕ϊ��e�[�u���p�G���g���B
 */
struct StringValueEntry {
    const char* name; // �l�ɑΉ����镶����
    uint32_t value; // �l
};
typedef std::vector<StringValueEntry> StringValueList;

/**
 * list�ɑ΂��āAstr����v����G���g�����������A��v�����l��Ԃ��B
 *
 * @param list ���X�g
 * @param str ������
 * @param pvalue �l���i�[����ϐ�
 * @retval true ���������ꍇ
 * @retval false ������Ȃ��ꍇ
 */
bool parse_value(const StringValueList& list, const std::string& str, uint32_t* pvalue);
/**
 * list�ɑ΂��āAstr����v����G���g�����������A��v�����l��Ԃ��B
 *
 * @param list ���X�g
 * @param str ������
 * @param pvalue �l���i�[����ϐ�
 * @retval true ���������ꍇ
 * @retval false ������Ȃ��ꍇ
 */
bool parse_value(const StringValueList& list, const char* str, uint32_t* pvalue);
/**
 * list�ɑ΂��āAvalue����v����G���g�����������A��v�����l��Ԃ��B
 * 
 * @param list ���X�g
 * @param value �l
 * @retval �C�e���[�^�B ���������ꍇ�͗L���ȃC�e���[�^�B������Ȃ��ꍇ�ɂ�list.end()��Ԃ��B
 */
StringValueList::const_iterator find_value(const StringValueList& list, uint32_t value);

/**
 * Windows�̃G���[���b�Z�[�W�𓾂�B
 *
 * @param error_code �G���[�R�[�h
 * @retval �G���[���b�Z�[�W
 */
const std::string get_windows_error_message(int error_code);
