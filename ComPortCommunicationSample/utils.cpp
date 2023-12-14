#include <Windows.h>
#include <shlwapi.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <list>

#include "utils.h"

#pragma comment(lib, "Shlwapi.lib")


bool parse_ui32(const std::string& str, uint32_t* pvalue) {
    return parse_ui32(str.c_str(), pvalue);
}

bool parse_ui32(const char* str, uint32_t* pvalue) {
    if ((str != nullptr) && (pvalue != nullptr)) {
        char* p;
        (*pvalue) = static_cast<uint32_t>(strtoul(str, &p, 0));
        return (p != nullptr) && (*p == '\0');
    }
    else {
        return false;
    }
}

bool parse_i32(const std::string& str, int32_t* pvalue) {
    return parse_i32(str.c_str(), pvalue);
}
bool parse_i32(const char* str, int32_t* pvalue) {
    if ((str != nullptr) && (pvalue != nullptr)) {
        char* p;
        (*pvalue) = static_cast<int32_t>(strtol(str, &p, 0));
        return (p != nullptr) && (*p == '\0');
    }
    else {
        return false;
    }
}


std::string strtrim(const std::string& str, const std::string& delim) {
    std::string::size_type begin_pos = str.find_first_not_of(delim);
    std::string::size_type tail_pos = str.find_last_not_of(delim);

    return str.substr(begin_pos, tail_pos - begin_pos);
}

char* strtrim(char* str, const char* delim) {
    if ((delim == nullptr) || (*delim == '\0')) {
        return str;
    }

    char* head = str;

    while ((*head != '\0') && (strchr(delim, *head) != nullptr)) {
        head++;
    }

    size_t len = strlen(head);
    while ((len > 0) && (strchr(delim, head[len - 1]) != nullptr)) {
        len--;
    }
    head[len] = '\0';

    return head;
}

void make_argv(char* str, argchar_t* pargv, const char* delim) {
    if ((str == nullptr) || (pargv == nullptr)) {
        return;
    }

    (*pargv).clear();

    char* p = str;
    while ((p != nullptr) && (*p != '\0')) {
        while ((p != nullptr) && (*p != '\0')  // pは末尾でない？
            && (strchr(delim, *p) != nullptr)) { // pはデリミタ？
            p++;
        }
        if (*p == '\0') {
            break;
        }
        char* begin = p;
        if ((*begin == '\'') // シングルクォーテーション？
            || (*begin == '"')) { // ダブルクォーテーション？
            char c = *begin;
            char* end = strchr(begin + 1, c); // 次のシングル/ダブルクォーテーションを探す
            if (end != nullptr) {
                *begin = '\0';
                begin++;
                (*pargv).push_back(begin);
                *end = '\0';
                p = end + 1;
            }
            else {
                p = begin + strlen(begin);
            }
        }
        else {
            while ((p != nullptr) && (*p != '\0') // 終端でない？
                && (strchr(delim, *p) == nullptr)) { // デリミタに不一致？
                p++;
            }
            (*pargv).push_back(begin);
            *p = '\0';
            p++;
        }
    }

    return;
}

void make_argv(const std::string& str, arg_t* pargv, const std::string& delim) {
    if (pargv == nullptr) {
        return;
    }

    (*pargv).clear();

    std::string::size_type pos = 0;
    std::string::size_type str_len = str.length();
    while (pos < str_len) {
        auto find_pos = str.find_first_not_of(delim, pos);
        if (find_pos == std::string::npos) {
            break;
        }
        auto begin_pos = find_pos;
        char begin_char = str.at(begin_pos);
        if ((begin_char == '\'') || (begin_char == '\"')) {
            auto tail_pos = str.find_first_of(begin_char, begin_pos);
            if (tail_pos != std::string::npos) {
                auto token = str.substr(begin_pos + 1, tail_pos - begin_pos - 2);
                (*pargv).push_back(token);
                pos = tail_pos + 1;
            }
            else {
                auto token = str.substr(begin_pos);
                (*pargv).push_back(token);
                pos += token.length();
            }
        }
        else {
            auto tail_pos = str.find_first_of(delim, begin_pos);
            if (tail_pos != std::string::npos) {
                auto token = str.substr(begin_pos, tail_pos - begin_pos);
                (*pargv).push_back(token);
                pos = tail_pos + 1;
            }
            else {
                auto token = str.substr(begin_pos);
                (*pargv).push_back(token);
                pos += token.length();
            }
        }
    }

    return;

}

std::string get_process_filename(void) {
    char path[MAX_PATH];
    DWORD size = GetModuleFileNameA(nullptr, path, sizeof(path));
    path[sizeof(path) - 1] = '\0';

    std::string filename(PathFindFileNameA(path));

    return filename;
}
bool parse_value(const StringValueList& list, const std::string& str, uint32_t* pvalue)
{
    return parse_value(list, str.c_str(), pvalue);
}

bool parse_value(const StringValueList& list, const char* str, uint32_t* pvalue) {
    if ((str == nullptr) || (pvalue == nullptr)) {
        return false;
    }

    auto it = find_if(list.begin(), list.end(), [str](const StringValueEntry& entry) { return strcmp(str, entry.name); });
    if (it == list.end()) {
        return false;
    }

    (*pvalue) = (*it).value;
    return true;
}

StringValueList::const_iterator find_value(const StringValueList& list, uint32_t value) {
    return std::find_if(list.begin(), list.end(), [value](const StringValueEntry& entry) { return entry.value == value; });
}

const std::string get_windows_error_message(int error_code) {
    LPSTR lpMsgBuf;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&lpMsgBuf), 0, NULL) == 0) {
        // Fail.
        return std::string("unknown error.");
    }
    else {
        std::string error_message(lpMsgBuf);
        LocalFree(lpMsgBuf);
        return error_message;
    }
}