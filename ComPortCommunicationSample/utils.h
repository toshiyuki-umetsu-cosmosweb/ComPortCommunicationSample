#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

/**
 * strを解析して符号無し32bit整数を得る。
 * 
 * @param str 文字列
 * @param pvalue 符号無し整数を格納する変数
 * @retval true 成功
 * @retval false 失敗
 */
bool parse_ui32(const std::string& str, uint32_t* pvalue);
/**
 * strを解析して符号無し32bit整数を得る。
 *
 * @param str 文字列
 * @param pvalue 符号無し整数を格納する変数
 * @retval true 成功
 * @retval false 失敗
 */
bool parse_ui32(const char* str, uint32_t* pvalue);
/**
 * strを解析して符号あり32bit整数を得る。
 *
 * @param str 文字列
 * @param pvalue 符号あり整数を格納する変数
 * @retval true 成功
 * @retval false 失敗
 */
bool parse_i32(const std::string& str, int32_t* pvalue);
/**
 * strを解析して符号あり32bit整数を得る。
 *
 * @param str 文字列
 * @param pvalue 符号あり整数を格納する変数
 * @retval true 成功
 * @retval false 失敗
 */
bool parse_i32(const char* str, int32_t* pvalue);

/**
 *  strの先頭と末尾にあるdelimに含まれる文字を除いて返す。
 *
 * @param str 文字列
 * @param delim デリミタ
 * @retval delimが除かれた文字列。
 */
std::string strtrim(const std::string& str, const std::string &delim = std::string(" \t\r\n"));
/**
 *  strの先頭と末尾にあるdelimに含まれる文字を除いて返す。
 *
 * @param str 文字列
 * @param delim デリミタ
 * @retval delimが除かれた文字列。
 */
char* strtrim(char* str, const char* delim = " \t\r\n");

typedef std::vector<char*> argchar_t;
/**
 * strをトークン分割する。
 *
 * @param str 文字列
 * @param pargv 分割したトークンを格納するリスト
 * @param delim デリミタ
 */
void make_argv(char* str, argchar_t* pargv, const char *delim = " \t\r\n");

typedef std::vector<std::string> arg_t;
/**
 * strをトークン分割する。
 *
 * @param str 文字列
 * @param pargv 分割したトークンを格納する配列
 * @param delim 分割に使用する区切り文字の集合
 */
void make_argv(const std::string& str, arg_t* pargv, const std::string &delim = std::string(" \t\r\n"));

/**
 * プロセスのファイル名を得る。
 *
 * Note: 深いディレクトリ(パス文字列がMAX_PATHを超えるケース)では、正しく動作しない。
 *
 * @retval プロセスのファイル名。
 */
std::string get_process_filename(void);


/**
 * 書式指定して文字列を生成する。
 * C++11用。
 * 
 * https://pyopyopyo.hatenablog.com/entry/2019/02/08/102456 を参考にさせていただいた。
 * Note: C++11用。 C++20 ではstd::format()があるのでそちらを使うべき。
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
 * 文字列 と 32bit符号無し整数 をペアにした値エントリ。
 * 外部(使用者)とのI/Fは文字列として、プログラム内部では数値で扱うための変換テーブル用エントリ。
 */
struct StringValueEntry {
    const char* name; // 値に対応する文字列
    uint32_t value; // 値
};
typedef std::vector<StringValueEntry> StringValueList;

/**
 * listに対して、strが一致するエントリを検索し、一致した値を返す。
 *
 * @param list リスト
 * @param str 文字列
 * @param pvalue 値を格納する変数
 * @retval true 見つかった場合
 * @retval false 見つからない場合
 */
bool parse_value(const StringValueList& list, const std::string& str, uint32_t* pvalue);
/**
 * listに対して、strが一致するエントリを検索し、一致した値を返す。
 *
 * @param list リスト
 * @param str 文字列
 * @param pvalue 値を格納する変数
 * @retval true 見つかった場合
 * @retval false 見つからない場合
 */
bool parse_value(const StringValueList& list, const char* str, uint32_t* pvalue);
/**
 * listに対して、valueが一致するエントリを検索し、一致した値を返す。
 * 
 * @param list リスト
 * @param value 値
 * @retval イテレータ。 見つかった場合は有効なイテレータ。見つからない場合にはlist.end()を返す。
 */
StringValueList::const_iterator find_value(const StringValueList& list, uint32_t value);

/**
 * Windowsのエラーメッセージを得る。
 *
 * @param error_code エラーコード
 * @retval エラーメッセージ
 */
const std::string get_windows_error_message(int error_code);
