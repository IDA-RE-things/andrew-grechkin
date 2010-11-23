﻿/** win_std.hpp
 *	@author		© 2010 Andrew Grechkin
 *	Source code: <http://code.google.com/p/andrew-grechkin>
 **/
#ifndef WIN_STD_HPP
#define WIN_STD_HPP

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#if (WINVER < 0x0501)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <stdint.h>
#include <windows.h>
#include <wchar.h>

#include <algorithm>

#ifdef	__x86_64__
#define nullptr 0ll
#else
#define nullptr 0
#endif

#define MAX_PATH_LEN		32772
#define STR_END				L'\0'
#define EMPTY				L""
#define SPACE				L" "
#define SPACE_C				L' '
#define PATH_SEPARATOR		L"\\" // Path separator in the file system
#define PATH_SEPARATOR_C	L'\\' // Path separator in the file system
#define PATH_PREFIX_NT		L"\\\\?\\" // Prefix to put ahead of a long path for Windows API
#define NET_PREFIX			L"\\\\"
#define NORM_M_PREFIX(m)	(*(LPDWORD)m==0x5c005c)
#define REV_M_PREFIX(m)		(*(LPDWORD)m==0x2f002f)

#define BOM_UTF32le			0x0000FEFF
#define BOM_UTF32be			0xFFFE0000
#define BOM_UTF16le			0xFEFF
#define BOM_UTF16be			0xFFFE
#define BOM_UTF8			0xBFBBEF

#define CP_UTF16le			1200
#define CP_UTF16be			1201
#define CP_UTF32le			1202
#define CP_UTF32be			1203
#define CP_AUTODETECT		((UINT)-1)
#define DEFAULT_CHAR_CP		CP_UTF8

#define NTSIGNATURE(a) ((LPVOID)((BYTE *)(a) + ((PIMAGE_DOS_HEADER)(a))->e_lfanew))

#define THIS_FILE ((strrchr(__FILE__, '\\') ?: __FILE__ - 1) + 1)

#ifndef sizeofa
#define sizeofa(array)		(sizeof(array)/sizeof(array[0]))
#endif

#ifndef sizeofe
#define sizeofe(array)		(sizeof(array[0]))
#endif

#ifndef S_IXUSR
#define S_IFDIR 0x4000
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001
#endif

typedef const void *PCVOID;

///====================================================================================== Uncopyable
/// Базовый класс для private наследования классами, объекты которых не должны копироваться
class Uncopyable {
	Uncopyable(const Uncopyable&);
	Uncopyable &operator=(const Uncopyable&);
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
};

///========================================================================================== WinMem
/// Функции работы с кучей
namespace WinMem {
	template<typename Type>
	inline bool Alloc(Type &in, size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		in = static_cast<Type> (::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS
		    | flags, size));
		return in != nullptr;
	}
	inline PVOID Alloc(size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		return ::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | flags, size);
	}

	template<typename Type>
	inline bool Realloc(Type &in, size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		if (in != nullptr)
			in = (Type)::HeapReAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS
			    | flags, (PVOID)in, size);
		else
			in = (Type)::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | flags, size);
		return in != nullptr;
	}

	template<typename Type>
	inline void Free(Type &in) {
		if (in) {
			::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
			in = nullptr;
		}
	}

	inline size_t Size(PCVOID in) {
		return (in) ? ::HeapSize(::GetProcessHeap(), 0, in) : 0;
	}

	inline bool Cmp(PCVOID m1, PCVOID m2, size_t size) {
		return ::memcmp(m1, m2, size) == 0;
	}

	inline PVOID Copy(PVOID dest, PCVOID sour, size_t size) {
		return ::memcpy(dest, sour, size);
	}
	inline PVOID Fill(PVOID in, size_t size, char fill) {
		return ::memset(in, (int)fill, size);
	}
	inline void Zero(PVOID in, size_t size) {
		Fill(in, size, 0);
	}
	template<typename Type>
	inline void Fill(Type &in, char fill) {
		Fill(&in, sizeof(in), fill);
	}
	template<typename Type>
	inline void Zero(Type &in) {
		Fill(&in, sizeof(in), 0);
	}
}

#ifdef NoStdNew
inline void* operator new(size_t size) {
	return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
inline void* operator new[](size_t size) {
	return ::operator new(size);
}
inline void operator delete(void *in) {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}
inline void operator delete[](void *ptr) {
	::operator delete(ptr);
}
#endif

inline void XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}

inline void XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

inline intmax_t Mega2Bytes(size_t in) {
	return (in != 0) ? (intmax_t)in << 20 : -1ll;
}

inline size_t Bytes2Mega(intmax_t in) {
	return (in > 0) ? in >> 20 : 0;
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_buf
///======================================================================================== auto_buf
template<typename Type>
class auto_buf {
public:
	typedef Type value_type;
	typedef size_t size_type;
	typedef auto_buf<Type> class_type;
	~auto_buf() {
		WinMem::Free(m_ptr);
	}
	auto_buf() :
		m_ptr(nullptr) {
	}
	auto_buf(size_type size) :
		m_ptr(WinMem::Alloc(size, 0)) {
	}
	void reserve(size_type nsize) {
		if (size() < nsize) {
			WinMem::Realloc(m_ptr, nsize);
		}
	}
	size_type size() const {
		return WinMem::Size(m_ptr);
	}
	void swap(value_type &ptr) {
		std::swap(m_ptr, ptr);
	}
	void swap(class_type &rhs) {
		std::swap(m_ptr, rhs.m_ptr);
	}
	operator value_type() const {
		return (value_type)m_ptr;
	}
private:
	auto_buf(const class_type &);
	auto_buf& operator=(const class_type &);
	value_type m_ptr;
};

template<typename Type>
void swap(auto_buf<Type> &b1, auto_buf<Type> &b2) {
	b1.swap(b2);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_free
///======================================================================================= auto_free
template<typename Type>
class auto_close: private Uncopyable {
public:
	typedef Type value_type;
	typedef auto_close<value_type> class_type;
	typedef void (*degenerate_function_type)(void*);
	~auto_close() {
		close();
	}
	template<typename Deleter>
	explicit auto_close(const value_type &ptr, Deleter del) :
		m_ptr(ptr), m_del((degenerate_function_type)del), m_free(true) {
	}
	operator value_type() {
		return m_ptr;
	}
	value_type* operator&() {
		return &m_ptr;
	}
	void close() {
		if (m_free) {
			m_del(m_ptr);
			m_free = false;
		}
	}
private:
	value_type m_ptr;
	degenerate_function_type m_del;
	bool	m_free;
};

template<>
class auto_close<HANDLE>: private Uncopyable {
public:
	typedef HANDLE value_type;
	typedef auto_close<value_type> class_type;
	~auto_close() {
		close();
	}
	explicit auto_close(const value_type &ptr = nullptr) :
		m_ptr(ptr) {
	}
	operator value_type() const {
		return (value_type)m_ptr;
	}
	value_type* operator&() {
		return &m_ptr;
	}
	void close() {
		if (m_ptr && m_ptr != INVALID_HANDLE_VALUE) {
			::CloseHandle(m_ptr);
			m_ptr = nullptr;
		}
	}
private:
	value_type m_ptr;
};

///========================================================================================= WinFlag
/// Проверка и установка битовых флагов
namespace WinFlag {
	template<typename Type>
	bool Check(Type in, Type flag) {
		return flag == (in & flag);
	}

	template<typename Type>
	bool CheckAny(Type in, Type flag) {
		return in & flag;
	}

	template<typename Type>
	Type &Set(Type &in, Type flag) {
		return in |= flag;
	}

	template<typename Type>
	Type &UnSet(Type &in, Type flag) {
		return in &= ~flag;
	}

	template<typename Type>
	Type &Switch(Type &in, Type flag, bool sw) {
		if (sw)
			return Set(in, flag);
		else
			return UnSet(in, flag);
	}
}

///========================================================================================== WinBit
//template<typename Type>
//struct type_size_bits {
//	size_t value = sizeof(Type) * 8;
//};

/// Проверка и установка битов
namespace WinBit {
	template<typename Type>
	size_t BIT_LIMIT() {
		return sizeof(Type) * 8;
	}

	template<typename Type>
	bool BadBit(size_t in) {
		return !(in < BIT_LIMIT<Type> ());
	}

	template<typename Type>
	size_t Limit(size_t in) {
		return (in == 0) ? BIT_LIMIT<Type> () : std::min<int>(in, BIT_LIMIT<Type> ());
	}

	template<typename Type>
	bool Check(Type in, size_t bit) {
		if (BadBit<Type> (bit))
			return false;
		Type tmp = 1;
		tmp <<= bit;
		return (in & tmp);
	}

	template<typename Type>
	Type &Set(Type &in, size_t bit) {
		if (BadBit<Type> (bit))
			return in;
		Type tmp = 1;
		tmp <<= bit;
		return (in |= tmp);
	}

	template<typename Type>
	Type &UnSet(Type &in, size_t bit) {
		if (BadBit<Type> (bit))
			return in;
		Type tmp = 1;
		tmp <<= bit;
		return (in &= ~tmp);
	}

	template<typename Type>
	Type &Switch(Type &in, size_t bit, bool sw) {
		if (sw)
			return Set(in, bit);
		else
			return UnSet(in, bit);
	}
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
	//	return	in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_SPACE);
}
inline bool IsPrint(WCHAR in) {
	return !WinFlag::Check(GetType(in), C1_CNTRL);
}
inline bool IsCntrl(WCHAR in) {
	//	return	in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_CNTRL);
}
inline bool IsUpper(WCHAR in) {
	//	return	::IsCharUpperW(in);
	return WinFlag::Check(GetType(in), C1_UPPER);
}
inline bool IsLower(WCHAR in) {
	//	return	::IsCharLowerW(in);
	return WinFlag::Check(GetType(in), C1_LOWER);
}
inline bool IsAlpha(WCHAR in) {
	//	return	::IsCharAlphaW(in);
	return WinFlag::Check(GetType(in), C1_ALPHA);
}
inline bool IsAlNum(WCHAR in) {
	//	return	::IsCharAlphaW(in);
	int Result = GetType(in);
	return WinFlag::Check(Result, C1_ALPHA) || WinFlag::Check(Result, C1_DIGIT);
}
inline bool IsDigit(WCHAR in) {
	//	return	::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_DIGIT);
}
inline bool IsXDigit(WCHAR in) {
	//	return	::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_XDIGIT);
}
inline bool IsPunct(WCHAR in) {
	//	return	::IsCharAlphaNumeric(in);
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
inline int Cmp(PCSTR in1, PCSTR in2) {
	return ::strcmp(in1, in2);
	//	return	::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int Cmp(PCSTR in1, PCSTR in2, size_t n) {
	return ::strncmp(in1, in2, n);
	//	return	::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int Cmp(PCWSTR in1, PCWSTR in2) {
	return ::wcscmp(in1, in2);
	//	return	::wcscoll(in1, in2);
	//	return	::CompareStringW(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
	//	return	::CompareStringW(LOCALE_USER_DEFAULT, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::wcsncmp(in1, in2, n);
	//	return	::CompareStringW(0 , NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int Cmpi(PCSTR in1, PCSTR in2) {
	//	return	::_stricmp(in1, in2);
	return ::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int Cmpi(PCSTR in1, PCSTR in2, size_t n) {
	return ::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
}
inline int Cmpi(PCWSTR in1, PCWSTR in2) {
	//	return	::_wcsicmp(in1, in2);
	//	return	::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
	//	return	fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
	return ::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int Cmpi(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
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
inline PSTR Find(PCSTR where, CHAR what) {
	return ::strchr(where, what);
}
inline PWSTR Find(PCWSTR where, PCWSTR what) {
	return ::wcsstr(where, what);
}
inline PWSTR Find(PCWSTR where, WCHAR what) {
	return ::wcschr(where, what);
}
inline PSTR RFind(PCSTR where, PCSTR what) {
	return ::strstr(where, what);
}
inline PSTR RFind(PCSTR where, CHAR what) {
	return ::strrchr(where, what);
}
inline PWSTR RFind(PCWSTR where, PCWSTR what) {
	return ::wcsstr(where, what);
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

inline PWSTR CharFirstOf(PCWSTR in, PCWSTR mask) {
	return (PWSTR)(in + ::wcscspn(in, mask));
}
inline PWSTR CharFirstNotOf(PCWSTR in, PCWSTR mask) {
	return (PWSTR)(in + ::wcsspn(in, mask));
}
inline PWSTR CharLastOf(PCWSTR in, PCWSTR mask) {
	size_t len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i - 1] == mask[j])
				return (PWSTR)&in[i - 1];
		}
	}
	return nullptr;
}
inline PWSTR CharLastNotOf(PCWSTR in, PCWSTR mask) {
	size_t len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i - 1] == mask[j])
				break;
			if (j == len - 1)
				return (PWSTR)&in[i - 1];
		}
	}
	return nullptr;
}

inline int64_t AsInt64(PCSTR in, int base = 10) {
	return _atoi64(in);
}
inline ULONG AsULong(PCSTR in, int base = 10) {
	PSTR end_ptr;
	return ::strtoul(in, &end_ptr, base);
}
inline long AsLong(PCSTR in, int base = 10) {
	PSTR end_ptr;
	return ::strtol(in, &end_ptr, base);
}
inline UINT AsUInt(PCSTR in, int base = 10) {
	return (unsigned int)AsULong(in, base);
}
inline int AsInt(PCSTR in, int base = 10) {
	return (int)AsLong(in, base);
}
inline double AsDouble(PCSTR in) {
	PSTR end_ptr;
	return ::strtod(in, &end_ptr);
}

inline int64_t AsInt64(PCWSTR in, int base = 10) {
	//	return	_wtoi64(in);
	PWSTR end_ptr;
	return ::wcstoll(in, &end_ptr, base);
}
inline uint64_t AsUInt64(PCWSTR in, int base = 10) {
	//	return	_wtoi64(in);
	PWSTR end_ptr;
	return ::wcstoull(in, &end_ptr, base);
}
inline ULONG AsULong(PCWSTR in, int base = 10) {
	PWSTR end_ptr;
	return ::wcstoul(in, &end_ptr, base);
}
inline long AsLong(PCWSTR in, int base = 10) {
	PWSTR end_ptr;
	return ::wcstol(in, &end_ptr, base);
}
inline UINT AsUInt(PCWSTR in, int base = 10) {
	return (unsigned int)AsULong(in, base);
}
inline int AsInt(PCWSTR in, int base = 10) {
	return (int)AsLong(in, base);
}
inline double AsDouble(PCWSTR in) {
	PWSTR end_ptr;
	return ::wcstod(in, &end_ptr);
}

//inline string	d2a(double in) {
//	CHAR	buf[MAX_PATH];
//	::_gcvt(in, 12, buf);
//	return	buf;
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
#include "win_autostr.h"
#else
#include "win_autoutf.h"
#endif

inline AutoUTF operator+(PCSTR lhs, const AutoUTF &rhs) {
	AutoUTF tmp(lhs);
	return tmp += rhs;
}

inline AutoUTF operator+(PCWSTR lhs, const AutoUTF &rhs) {
	AutoUTF tmp(lhs);
	return tmp += rhs;
}

inline size_t Len(const CStrA &in) {
	return in.size();
}
inline size_t Len(const AutoUTF &in) {
	return in.size();
}

inline CStrA oem(PCWSTR in) {
	return w2cp(in, CP_OEMCP);
}
inline CStrA oem(const AutoUTF &in) {
	return w2cp(in.c_str(), CP_OEMCP);
}
inline CStrA utf8(PCWSTR in) {
	return w2cp(in, CP_UTF8);
}
inline CStrA utf8(const AutoUTF &in) {
	return w2cp(in.c_str(), CP_UTF8);
}
inline AutoUTF utf16(PCSTR in) {
	return cp2w(in, CP_UTF16le);
}
inline AutoUTF utf16(const CStrA &in) {
	return cp2w(in.c_str(), CP_UTF16le);
}

inline PCSTR Num2StrA(PSTR str, int64_t num, int base = 10) {
	return ::_i64toa(num, str, base); //lltoa
}

inline PCWSTR Num2Str(PWSTR str, int64_t num, int base = 10) {
	return ::_i64tow(num, str, base); //lltow
}

inline CStrA Num2StrA(int64_t num, int base = 10) {
	CHAR buf[64];
	Num2StrA(buf, num, base);
	return CStrA(buf);
}

inline AutoUTF Num2Str(int64_t num, int base = 10) {
	WCHAR buf[64];
	Num2Str(buf, num, base);
	return AutoUTF(buf);
}

inline AutoUTF ErrAsStr(HRESULT err = ::GetLastError(), PCWSTR lib = nullptr) {
	HMODULE mod = nullptr;
	if (err && lib) {
		mod = ::LoadLibraryExW(lib, nullptr, DONT_RESOLVE_DLL_REFERENCES); //LOAD_LIBRARY_AS_DATAFILE
	}
	PWSTR buf = nullptr;
	::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | ((mod) ? FORMAT_MESSAGE_FROM_HMODULE
	                                                         : FORMAT_MESSAGE_FROM_SYSTEM), mod,
	                 err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //GetSystemDefaultLangID(),
	                 (PWSTR)&buf, 0, nullptr);
	AutoUTF Result((buf) ? buf : L"Unknown error\r\n");
	::LocalFree(buf);
	Result.erase(Result.size() - 2);
	if (mod)
		::FreeLibrary(mod);
	return Result;
}

inline AutoUTF ErrWmiAsStr(HRESULT err) {
	return ErrAsStr(err, L"wmiutils.dll");
}

inline CStrA& Trim_l(CStrA &str, const CStrA &chrs = " \t\r\n") {
	CStrA::size_type pos = str.find_first_not_of(chrs);
	if (pos && pos != CStrA::npos) {
		str.erase(0, pos);
	}
	return str;
}

inline CStrA& Trim_r(CStrA &str, const CStrA &chrs = " \t\r\n") {
	CStrA::size_type pos = str.find_last_not_of(chrs);
	if (pos != CStrA::npos && (++pos < str.size())) {
		str.erase(pos);
	}
	return str;
}

inline CStrA& Trim(CStrA &str, const CStrA &chrs = " \t\r\n") {
	Trim_r(str, chrs);
	Trim_l(str, chrs);
	return str;
}

inline CStrA TrimOut(const CStrA &str, const CStrA &chrs = " \t\r\n") {
	CStrA tmp(str);
	return Trim(tmp, chrs);
}

inline AutoUTF& Trim_l(AutoUTF &str, const AutoUTF &chrs = L" \t\r\n") {
	AutoUTF::size_type pos = str.find_first_not_of(chrs);
	if (pos && pos != AutoUTF::npos) {
		str.erase(0, pos);
	}
	return str;
}

inline AutoUTF& Trim_r(AutoUTF &str, const AutoUTF &chrs = L" \t\r\n") {
	AutoUTF::size_type pos = str.find_last_not_of(chrs);
	if (pos != AutoUTF::npos && (++pos < str.size())) {
		str.erase(pos);
	}
	return str;
}

inline AutoUTF& Trim(AutoUTF &str, const AutoUTF &chrs = L" \t\r\n") {
	Trim_r(str, chrs);
	Trim_l(str, chrs);
	return str;
}

inline AutoUTF TrimOut(const AutoUTF &str, const AutoUTF &chrs = L" \t\r\n") {
	AutoUTF tmp(str);
	return Trim(tmp, chrs);
}

inline AutoUTF GetWord(const AutoUTF &str, WCHAR d = PATH_SEPARATOR_C) {
	AutoUTF::size_type pos = str.find(d);
	if (pos != AutoUTF::npos)
		return str.substr(0, pos);
	return str;
}

inline CStrA& AddWord(CStrA &inout, const CStrA &add, const CStrA &delim = "") {
	CStrA::size_type pos = inout.size() - delim.size();
	if (!(delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return inout;
}

inline AutoUTF& AddWord(AutoUTF &inout, const AutoUTF &add, const AutoUTF &delim = L"") {
	// добаваляет строку через разделитель кроме случаев
	// 1) исходная строка пуста
	// 2) если разделитель есть в конце исходной строки
	// 3) если разделитель есть в начале добавляемой
	AutoUTF::size_type pos = inout.size() - delim.size();
	if (!(delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return inout;
}

inline CStrA& AddWordEx(CStrA &inout, const CStrA &add, const CStrA &delim = "") {
	CStrA::size_type pos = inout.size() - delim.size();
	if (!(add.empty() || delim.empty() || inout.empty() || (inout.rfind(delim) == pos)
	    || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return inout;
}

inline AutoUTF& AddWordEx(AutoUTF &inout, const AutoUTF &add, const AutoUTF &delim = L"") {
	// добаваляет строку через разделитель кроме случаев
	// 1) исходная строка пуста
	// 2) если добавляемая строка пуста
	// 3) если разделитель есть в конце исходной строки
	// 4) если разделитель есть в начале добавляемой
	AutoUTF::size_type pos = inout.size() - delim.size();
	if (!(add.empty() || delim.empty() || inout.empty() || (inout.rfind(delim) == pos)
	    || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return inout;
}

inline CStrA CutWord(CStrA &inout, const CStrA &delim = "\t ", bool delDelim = true) {
	CStrA::size_type pos = inout.find_first_of(delim);
	CStrA Result(inout.substr(0, pos));
	if (delDelim && pos != CStrA::npos)
	//	pos = inout.find_first_not_of(delim, pos);
		++pos;
	inout.erase(0, pos);
	Trim_l(inout);
	return Trim(Result);
}

inline AutoUTF CutWord(AutoUTF &inout, const AutoUTF &delim = L"\t ", bool delDelim = true) {
	AutoUTF::size_type pos = inout.find_first_of(delim);
	AutoUTF Result(inout.substr(0, pos));
	if (delDelim && pos != AutoUTF::npos) {
		//		pos = inout.find_first_not_of(delim, pos);
		++pos;
	}
	inout.erase(0, pos);
	return Trim(Result);
}

inline CStrA CutWordEx(CStrA &inout, const CStrA &delim, bool delDelim = true) {
	CStrA::size_type pos = inout.find(delim);
	CStrA Result = inout.substr(0, pos);
	if (delDelim && pos != CStrA::npos)
		pos += delim.size();
	inout.erase(0, pos);
	return Trim(Result);
}

inline AutoUTF CutWordEx(AutoUTF &inout, const AutoUTF &delim, bool delDelim = true) {
	AutoUTF::size_type pos = inout.find(delim);
	AutoUTF Result = inout.substr(0, pos);
	if (delDelim && pos != AutoUTF::npos)
		pos += delim.size();
	inout.erase(0, pos);
	return Trim(Result);
}

inline AutoUTF& ReplaceAll(AutoUTF& str, const AutoUTF &from, const AutoUTF &to) {
	AutoUTF::size_type pos;
	while ((pos = str.find(from)) != AutoUTF::npos) {
		str.replace(pos, from.size(), to);
	}
	return str;
}

inline AutoUTF ReplaceAllOut(const AutoUTF& str, const AutoUTF &from, const AutoUTF &to) {
	AutoUTF Result(str);
	return ReplaceAll(Result, from, to);
}

inline void mbox(PCSTR text, PCSTR capt = "") {
	::MessageBoxA(nullptr, text, capt, MB_OK);
}

inline void mbox(PCWSTR text, PCWSTR capt = L"") {
	::MessageBoxW(nullptr, text, capt, MB_OK);
}

inline void mbox(HRESULT err, PCWSTR lib = nullptr) {
	::MessageBoxW(nullptr, ErrAsStr(err, lib).c_str(), L"Error", MB_OK);
}

///=========================================================================================== DEBUG
//#ifdef DEBUG
//#include <iostream>
//#include <ostream>
//using std::cout;
//using std::ostream;
//using std::endl;
//
//inline ostream &operator<<(ostream &s, PCWSTR rhs) {
//	return s << utf8(rhs).c_str();
//}
//inline ostream &operator<<(ostream &s, const AutoUTF &rhs) {
//	return s << utf8(rhs).c_str();
//}
//#endif

#endif //WIN_STD_HPP