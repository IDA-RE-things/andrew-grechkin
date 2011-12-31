﻿/** str.hpp
 *	@author		© 2011 Andrew Grechkin
 **/

#ifndef _WIN_DEF_STR_HPP
#define _WIN_DEF_STR_HPP

#include "std.h"
#include "bit.h"
#include "memory.h"

extern "C" {
	long long __MINGW_NOTHROW	wcstoll(const wchar_t * __restrict__, wchar_t** __restrict__, int);
	unsigned long long __MINGW_NOTHROW wcstoull(const wchar_t * __restrict__, wchar_t ** __restrict__, int);
}

///====================================================================== Функции работы с символами
inline int GetType(WCHAR in) {
	WORD Result[2] = {0};
	::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
	return Result[0];
}

inline bool IsEol(WCHAR in) {
	return in == L'\r' || in == L'\n';
}

inline bool IsSpace(WCHAR in) {
	//	return in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_SPACE);
}

inline bool IsPrint(WCHAR in) {
	return !WinFlag::Check(GetType(in), C1_CNTRL);
}

inline bool IsCntrl(WCHAR in) {
	//	return in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_CNTRL);
}

inline bool IsUpper(WCHAR in) {
	//	return ::IsCharUpperW(in);
	return WinFlag::Check(GetType(in), C1_UPPER);
}

inline bool IsLower(WCHAR in) {
	//	return ::IsCharLowerW(in);
	return WinFlag::Check(GetType(in), C1_LOWER);
}

inline bool IsAlpha(WCHAR in) {
	//	return ::IsCharAlphaW(in);
	return WinFlag::Check(GetType(in), C1_ALPHA);
}

inline bool IsAlNum(WCHAR in) {
	//	return ::IsCharAlphaW(in);
	int Result = GetType(in);
	return WinFlag::Check(Result, C1_ALPHA) || WinFlag::Check(Result, C1_DIGIT);
}

inline bool IsDigit(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_DIGIT);
}

inline bool IsXDigit(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_XDIGIT);
}

inline bool IsPunct(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_PUNCT);
}

///====================================================================== Функции работы со строками
inline size_t Len(PCSTR in) {
	return ::strlen(in);
}

inline size_t Len(PCWSTR in) {
	return ::wcslen(in);
}

inline bool Empty(PCSTR in) {
	return in[0] == 0;
}

inline bool Empty(PCWSTR in) {
	return in[0] == 0;
}

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif
inline int CmpCode(PCSTR in1, PCSTR in2) {
	return ::strcmp(in1, in2);
}

inline int CmpCode(PCSTR in1, PCSTR in2, size_t n) {
	return ::strncmp(in1, in2, n);
}

inline int CmpCode(PCWSTR in1, PCWSTR in2) {
	return ::wcscmp(in1, in2);
	//	return ::wcscoll(in1, in2);
}

inline int CmpCode(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::wcsncmp(in1, in2, n);
}

inline int Cmp(PCSTR in1, PCSTR in2) {
	return ::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmp(PCSTR in1, PCSTR in2, size_t n) {
	return ::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}

inline int Cmp(PCWSTR in1, PCWSTR in2) {
	return ::CompareStringW(0 , SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::CompareStringW(0 , NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}

inline int Cmpi(PCSTR in1, PCSTR in2) {
	//	return ::_stricmp(in1, in2);
	return ::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmpi(PCSTR in1, PCSTR in2, size_t n) {
	return ::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
}

inline int Cmpi(PCWSTR in1, PCWSTR in2) {
	//	return ::_wcsicmp(in1, in2);
	//	return ::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
	//	return fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
	return ::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmpi(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
}

inline bool EqCode(PCSTR in1, PCSTR in2) {
	return CmpCode(in1, in2) == 0;
}

inline bool EqCode(PCWSTR in1, PCWSTR in2) {
	return CmpCode(in1, in2) == 0;
}

inline bool Eq(PCSTR in1, PCSTR in2) {
	return Cmp(in1, in2) == 0;
}

inline bool Eq(PCWSTR in1, PCWSTR in2) {
	return Cmp(in1, in2) == 0;
}

inline bool Eqi(PCSTR in1, PCSTR in2) {
	return Cmpi(in1, in2) == 0;
}

inline bool Eqi(PCWSTR in1, PCWSTR in2) {
	return Cmpi(in1, in2) == 0;
}

inline PSTR Copy(PSTR dest, PCSTR src) {
	return ::strcpy(dest, src);
}

inline PWSTR Copy(PWSTR dest, PCWSTR src) {
	return ::wcscpy(dest, src);
}

inline PSTR Copy(PSTR dest, PCSTR src, size_t size) {
	return ::strncpy(dest, src, size);
}

inline PWSTR Copy(PWSTR dest, PCWSTR src, size_t size) {
	return ::wcsncpy(dest, src, size);
}

inline PSTR Cat(PSTR dest, PCSTR src) {
	return ::strcat(dest, src);
}

inline PWSTR Cat(PWSTR dest, PCWSTR src) {
	return ::wcscat(dest, src);
}

inline PSTR Cat(PSTR dest, PCSTR src, size_t size) {
	return ::strncat(dest, src, size);
}

inline PWSTR Cat(PWSTR dest, PCWSTR src, size_t size) {
	return ::wcsncat(dest, src, size);
}

inline PSTR Find(PCSTR where, PCSTR what) {
	return ::strstr(where, what);
}

inline PCSTR Find(PCSTR where, CHAR what) {
	return ::strchr(where, what);
}

inline PWSTR Find(PCWSTR where, PCWSTR what) {
	return ::wcsstr(where, what);
}

inline PCWSTR Find(PCWSTR where, WCHAR what) {
	return ::wcschr(where, what);
}

inline PSTR RFind(PCSTR where, PCSTR what) {
	PCSTR last1 = where + Len(where);
	PCSTR last2 = what + Len(what);
	last2 = std::find_end(where, last1, what, last2);
	return (last1 == last2) ? nullptr : const_cast<PSTR>(last2);
}

inline PSTR RFind(PCSTR where, CHAR what) {
	return ::strrchr(where, what);
}

inline PWSTR RFind(PCWSTR where, PCWSTR what) {
	PCWSTR last1 = where + Len(where);
	PCWSTR last2 = what + Len(what);
	last2 = std::find_end(where, last1, what, last2);
	return (last1 == last2) ? nullptr : const_cast<PWSTR>(last2);
}

inline PWSTR RFind(PCWSTR where, WCHAR what) {
	return ::wcsrchr(where, what);
}

inline size_t Span(PCSTR str, PCSTR strCharSet) {
	return ::strcspn(str, strCharSet);
}

inline size_t Span(PCWSTR str, PCWSTR strCharSet) {
	return ::wcscspn(str, strCharSet);
}

inline int64_t AsInt64(PCSTR in) {
	return _atoi64(in);
}

inline unsigned int AsUInt(PCSTR in, int base = 10) {
	PSTR end_ptr;
	return ::strtoul(in, &end_ptr, base);
}

inline int AsInt(PCSTR in, int base = 10) {
	PSTR end_ptr;
	return ::strtol(in, &end_ptr, base);
}

inline double AsDouble(PCSTR in) {
	PSTR end_ptr;
	return ::strtod(in, &end_ptr);
}

inline uint64_t AsUInt64(PCWSTR in, int base = 10) {
	//	return _wtoi64(in);
	PWSTR end_ptr;
	return ::wcstoull(in, &end_ptr, base);
}

inline int64_t AsInt64(PCWSTR in, int base = 10) {
	//	return _wtoi64(in);
	PWSTR end_ptr;
	return ::wcstoll(in, &end_ptr, base);
}

inline unsigned int  AsUInt(PCWSTR in, int base = 10) {
	PWSTR end_ptr;
	return ::wcstoul(in, &end_ptr, base);
}

inline int AsInt(PCWSTR in, int base = 10) {
	PWSTR end_ptr;
	return ::wcstol(in, &end_ptr, base);
}

inline double AsDouble(PCWSTR in) {
	PWSTR end_ptr;
	return ::wcstod(in, &end_ptr);
}

inline PCSTR Num2Str(PSTR str, int64_t num, int base = 10) {
	return ::_i64toa(num, str, base); //lltoa
}

inline PCWSTR Num2Str(PWSTR str, int64_t num, int base = 10) {
	return ::_i64tow(num, str, base); //lltow
}

//inline string	d2a(double in) {
//	CHAR	buf[MAX_PATH];
//	::_gcvt(in, 12, buf);
//	return buf;
//}

inline WCHAR ToUpper(WCHAR in) {
	::CharUpperBuffW(&in, 1);
	return in;
}

inline WCHAR ToLower(WCHAR in) {
	::CharLowerBuffW(&in, 1);
	return in;
}

inline PWSTR ToUpper(PWSTR buf, size_t len) {
	::CharUpperBuffW(buf, len);
	return buf;
}

inline PWSTR ToLower(PWSTR buf, size_t len) {
	::CharLowerBuffW(buf, len);
	return buf;
}

inline PWSTR ToUpper(PWSTR s1) {
	return ToUpper(s1, Len(s1));
}

inline PWSTR ToLower(PWSTR s1) {
	return ToLower(s1, Len(s1));
}

inline PSTR Fill(PSTR in, CHAR ch) {
	return ::_strset(in, ch);
}

inline PWSTR Fill(PWSTR in, WCHAR ch) {
	return ::_wcsset(in, ch);
}

inline PSTR Reverse(PSTR in) {
	return ::_strrev(in);
}

inline PWSTR Reverse(PWSTR in) {
	return ::_wcsrev(in);
}

inline PWSTR AssignStr(PCWSTR src) {
	size_t len = Len(src) + 1;
	PWSTR dest;
	WinMem::Alloc(dest, len * sizeof(WCHAR));
	Copy(dest, src, len);
	return dest;
}

inline size_t Convert(PCSTR from, UINT cp, PWSTR to = nullptr, size_t size = 0) {
	return ::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
}

inline size_t Convert(PCWSTR from, UINT cp, PSTR to = nullptr, size_t size = 0) {
	return ::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, nullptr, nullptr);
}

#ifdef NoStlString
#include "autostr.h"
#else
#include "autoutf.h"
#endif

inline size_t Len(const astring &in) {
	return in.size();
}
inline size_t Len(const ustring &in) {
	return in.size();
}

inline astring Num2StrA(int64_t num, int base = 10) {
	CHAR buf[64];
	Num2Str(buf, num, base);
	return astring(buf);
}

inline ustring Num2Str(int64_t num, int base = 10) {
	WCHAR buf[64];
	Num2Str(buf, num, base);
	return ustring(buf);
}

inline astring oem(PCWSTR in) {
	return w2cp(in, CP_OEMCP);
}
inline astring oem(const ustring &in) {
	return w2cp(in.c_str(), CP_OEMCP);
}

inline astring ansi(PCWSTR in) {
	return w2cp(in, CP_ACP);
}
inline astring ansi(const ustring &in) {
	return w2cp(in.c_str(), CP_ACP);
}

inline astring utf8(PCWSTR in) {
	return w2cp(in, CP_UTF8);
}
inline astring utf8(const ustring &in) {
	return w2cp(in.c_str(), CP_UTF8);
}

inline ustring utf16(PCSTR in, UINT cp = CP_UTF8) {
	return cp2w(in, cp);
}
inline ustring utf16(const astring &in, UINT cp = CP_UTF8) {
	return cp2w(in.c_str(), cp);
}

astring& Trim_l(astring &str, const astring &chrs = " \t\r\n");

astring& Trim_r(astring &str, const astring &chrs = " \t\r\n");

astring& Trim(astring &str, const astring &chrs = " \t\r\n");

astring TrimOut(const astring &str, const astring &chrs = " \t\r\n");

ustring& Trim_l(ustring &str, const ustring &chrs = L" \t\r\n");

ustring& Trim_r(ustring &str, const ustring &chrs = L" \t\r\n");

ustring& Trim(ustring &str, const ustring &chrs = L" \t\r\n");

ustring TrimOut(const ustring &str, const ustring &chrs = L" \t\r\n");

ustring GetWord(const ustring &str, WCHAR d = PATH_SEPARATOR_C);

astring& AddWord(astring &inout, const astring &add, const astring &delim = "");

ustring& AddWord(ustring &inout, const ustring &add, const ustring &delim = L"");

astring& AddWordEx(astring &inout, const astring &add, const astring &delim = "");

ustring& AddWordEx(ustring &inout, const ustring &add, const ustring &delim = L"");

astring CutWord(astring &inout, const astring &delim = "\t ", bool delDelim = true);

ustring CutWord(ustring &inout, const ustring &delim = L"\t ", bool delDelim = true);

astring CutWordEx(astring &inout, const astring &delim, bool delDelim = true);

ustring CutWordEx(ustring &inout, const ustring &delim, bool delDelim = true);

ustring& ReplaceAll(ustring& str, const ustring &from, const ustring &to);

ustring ReplaceAllOut(const ustring& str, const ustring &from, const ustring &to);

inline void mbox(PCSTR text, PCSTR capt = "") {
	::MessageBoxA(nullptr, text, capt, MB_OK);
}

inline void mbox(PCWSTR text, PCWSTR capt = L"") {
	::MessageBoxW(nullptr, text, capt, MB_OK);
}

template<typename Type>
void StrToCont(const ustring &src, Type dst, const ustring &delim = L" \t\n\r") {
	ustring::size_type start, end = 0;
	while ((start = src.find_first_not_of(delim, end)) != ustring::npos) {
		end = src.find_first_of(delim, start);
		dst = src.substr(start, end - start);
	}
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ err
ustring ErrAsStr(DWORD err = ::GetLastError(), PCWSTR lib = nullptr);

inline ustring ErrWmiAsStr(HRESULT err) {
	return ErrAsStr(err, L"wmiutils.dll");
}

#include <tr1/functional>
using std::tr1::placeholders::_1;
using std::tr1::placeholders::_2;

template<typename Type>
const Type* find_first_of(const Type *where, const Type *what) {
	//	return (PWSTR)(in + ::wcscspn(in, mask));
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	str_t last1 = &where[Len(where)];
	str_t pos = find_if(&where[0], last1, tr1::bind((func_t)Find, what, _1));
	return (last1 == pos) ? nullptr : pos;
}
template<typename Type>
const Type* find_first_not_of(const Type *where, const Type *what) {
	//	return (PWSTR)(in + ::wcsspn(in, mask));
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	str_t last1 = &where[Len(where)];
	str_t pos = find_if(&where[0], last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
	return (last1 == pos) ? nullptr : pos;
}

template<typename Type>
const Type* find_last_of(const Type *where, const Type *what) {
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	reverse_iterator<str_t> first1(&where[Len(where)]);
	reverse_iterator<str_t> last1(&where[0]);
	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind((func_t)Find, what, _1));
//	reverse_iterator<str_t> pos = find_if(first1, last1, bind1st(ptr_fun<str_t, Type, str_t>(Find), what));
	return (last1 == pos) ? nullptr : &(*pos);
}
template<typename Type>
const Type* find_last_not_of(const Type *where, const Type *what) {
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	reverse_iterator<str_t> first1(&where[Len(where)]);
	reverse_iterator<str_t> last1(&where[0]);
	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
//	reverse_iterator<str_t> pos = find_if(first1, last1, not1(bind1st(ptr_fun<str_t, Type, str_t>(Find), what)));
	return (last1 == pos) ? nullptr : &(*pos);
}

#endif
