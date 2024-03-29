﻿#include <libbase/std.hpp>
#include <libbase/path.hpp>
#include <libbase/str.hpp>
#include <libbase/win64.hpp>

namespace Base {

extern "C" {
	BOOL WINAPI SHGetSpecialFolderPathW(HWND, LPWSTR, int, BOOL);
}

	ustring MakePath(PCWSTR path, PCWSTR name) {
		ustring Result(PathNice(SlashAdd(path)));
		return AddWordEx(Result, name, PATH_SEPARATOR);
	}

	ustring Expand(PCWSTR path) {
		wchar_t ret[MAX_PATH_LEN];
		return ::ExpandEnvironmentStringsW(path, ret, sizeofa(ret)) ? ustring(ret) : ustring();
	}

	ustring MakeGoodPath(PCWSTR path) {
		return get_fullpath(Expand(path));
	}

	ustring get_fullpath(PCWSTR path) {
		wchar_t buf[MAX_PATH_LEN];
		::GetFullPathNameW(path, sizeofa(buf), buf, nullptr);
		return ensure_path_prefix(ustring(buf));
	}

	ustring PathNice(PCWSTR path) {
		return Canonicalize(Expand(path));
	}

	ustring& ensure_end_path_separator(ustring &path, wchar_t sep) {
		if (!path.empty() && !Str::find(PATH_SEPARATORS, path[path.size() - 1])) {
			path += sep;
		}
		return path;
	}

	ustring& ensure_no_end_path_separator(ustring &path) {
		if (!path.empty() && Str::find(PATH_SEPARATORS, path[path.size() - 1])) {
			path.erase(path.size() - 1);
		}
		return path;
	}

	ustring SlashAdd(const ustring &path, wchar_t sep) {
		ustring ret(path);
		return ensure_end_path_separator(ret, sep);
	}

	ustring SlashDel(const ustring &path) {
		ustring ret(path);
		return ensure_no_end_path_separator(ret);
	}

	bool IsPathUnix(PCWSTR path) {
		return Str::find(path, L'/') != nullptr;
	}

	ustring ExtractFile(const ustring &path) {
		return path.substr(path.find_last_of(PATH_SEPARATORS));
	}

	ustring ExtractPath(const ustring &path) {
		return path.substr(0, path.find_last_of(PATH_SEPARATORS));
	}

	ustring PathUnix(PCWSTR path) {
		ustring Result(path);
		return ReplaceAll(Result, L"\\", L"/");
	}

	ustring PathWin(PCWSTR path) {
		ustring Result(path);
		return ReplaceAll(Result, L"/", L"\\");
	}

	ustring Secure(PCWSTR path) {
		ustring Result(path);
		ReplaceAll(Result, L"..", L"");
		ReplaceAll(Result, L"%", L"");
		return Validate(Result);
	}

	ustring Validate(PCWSTR path) {
		ustring Result(Canonicalize(Expand(path)));
		ReplaceAll(Result, L"...", L"");
		ReplaceAll(Result, L"\\\\", L"\\");
		if (Result == L"\\")
			Result.clear();
		return Result;
	}

	ustring GetSpecialPath(int csidl, bool create) {
		auto_array<wchar_t> ret(MAX_PATH_LEN);
		return SHGetSpecialFolderPathW(nullptr, ret.data(), csidl, create) ? ustring(ret.data()) : ustring();
	}

	ustring GetWorkDirectory() {
		wchar_t Result[::GetCurrentDirectoryW(0, nullptr)];
		::GetCurrentDirectoryW(sizeofa(Result), Result);
		return Result;
	}

	bool SetWorkDirectory(PCWSTR path) {
		return Str::is_empty(path) ? false : ::SetCurrentDirectoryW(path);
	}

	ustring TempDir() {
		wchar_t buf[MAX_PATH];
		buf[0] = 0;
		::GetTempPathW(sizeofa(buf), buf);
		return ustring(buf);
	}

	ustring TempFile(PCWSTR path) {
		wchar_t buf[MAX_PATH];
		wchar_t pid[32];
		buf[0] = 0;
		Str::convert_num(pid, ::GetCurrentProcessId());
		::GetTempFileNameW(path, pid, 0, buf);
		return buf;
	}

	bool is_path_mask(PCWSTR path) {
		PCWSTR pos = nullptr;
		if ((pos = Str::find(path, L'*')) || (pos = Str::find(path, L'?')))
			return pos != (path + 2);
		return false;
	}

	bool is_valid_filename(PCWSTR name) {
		return !(Str::compare(name, L".") || Str::compare(name, L"..") || Str::compare(name, L"..."));
	}

	ustring remove_path_prefix(const ustring &path, PCWSTR pref) {
		if (!path.empty() && path.find(pref) == 0)
			return path.substr(Str::length(pref));
		return path;
	}

	ustring ensure_path_prefix(const ustring &path, PCWSTR pref) {
		if (path.size() > 1 && (path[0] != L'\\' || path[1] != L'\\') && path.find(pref) == ustring::npos)
			return ustring(pref) += path;
		return path;
	}

	ustring get_path_from_mask(const ustring &mask) {
		ustring tmp(remove_path_prefix(mask));
		ustring::size_type pos = std::min(tmp.find_first_of(L"?*"), tmp.find_last_of(L"\\/"));
		if (pos != ustring::npos) {
			tmp.erase(pos);
		}
		return ensure_path_prefix(tmp);
	}

	ustring get_root(PCWSTR path) {
		wchar_t ret[MAX_PATH];
		if (::GetVolumePathNameW(path, ret, sizeofa(ret)))
			return ustring(ret);
		return ustring(path);
	}

	///========================================================================================= SysPath
	namespace SysPath {
		ustring Winnt() {
			const int CSIDL_WINDOWS = 0x0024;
			ustring ret(GetSpecialPath(CSIDL_WINDOWS));
			return ensure_end_path_separator(ret);
		}
		ustring Sys32() {
			const int CSIDL_SYSTEM = 0x0025;
			ustring ret(GetSpecialPath(CSIDL_SYSTEM));
			return ensure_end_path_separator(ret);
		}

		ustring SysNative() {
			return PathNice(is_WOW64() ? L"%SystemRoot%\\sysnative\\" : L"%SystemRoot%\\system32\\");
		}

		ustring InetSrv() {
			return MakePath(Sys32().c_str(), L"inetsrv\\");
		}

		ustring Dns() {
			return MakePath(SysNative().c_str(), L"dns\\");
		}

		ustring Temp() {
			return PathNice(L"%TEMP%\\");
		}

		ustring Users() {
			ustring ret = PathNice(L"%PUBLIC%\\..\\");
			return (ret.empty() || (ret == L"\\")) ? PathNice(L"%ALLUSERSPROFILE%\\..\\") : ret;
		}
	}

	///========================================================================================== SysApp
	namespace SysApp {
		ustring appcmd() {
			return SysPath::InetSrv() + L"appcmd.exe ";
		}
	}

	///=================================================================================================
	bool substr_match(const ustring& str, size_t pos, PCWSTR mstr) {
		size_t mstr_len = Str::length(mstr);
		if ((pos > str.size()) || (pos + mstr_len > str.size())) {
			return false;
		}
		return wmemcmp(str.c_str() + pos, mstr, mstr_len) == 0;
	}

	void locate_path_root(const ustring& path, size_t& path_root_len, bool& is_unc_path) {
		unsigned prefix_len = 0;
		is_unc_path = false;
		if (substr_match(path, 0, L"\\\\")) {
			if (substr_match(path, 2, L"?\\UNC\\")) {
				prefix_len = 8;
				is_unc_path = true;
			} else if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
				prefix_len = 4;
			} else {
				prefix_len = 2;
				is_unc_path = true;
			}
		}
		if ((prefix_len == 0) && !substr_match(path, 1, L":\\")) {
			path_root_len = 0;
		} else {
			ustring::size_type p = path.find(L'\\', prefix_len);
			if (p == ustring::npos) {
				p = path.size();
			}
			if (is_unc_path) {
				p = path.find(L'\\', p + 1);
				if (p == ustring::npos) {
					p = path.size();
				}
			}
			path_root_len = p;
		}
	}

	ustring extract_file_name(const ustring& path) {
		size_t pos = path.rfind(L"\\");
		if (pos == ustring::npos) {
			pos = 0;
		} else {
			pos++;
		}
		size_t path_root_len;
		bool is_unc_path;
		locate_path_root(path, path_root_len, is_unc_path);
		if ((pos <= path_root_len) && (path_root_len != 0))
			return ustring();
		else
			return path.substr(pos);
	}

}
