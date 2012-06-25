﻿#include <libbase/logger.hpp>

#include <libbase/lock.hpp>

namespace Base {
	namespace Logger {

		struct LogToFile: public Target_i {
			virtual ~LogToFile();

			virtual void out(const Logger_i * lgr, Level lvl, PCWSTR str, size_t size) const;

			LogToFile(PCWSTR path);

		private:
			HANDLE m_file;
			Base::SyncUnit_i * m_sync;
		};

		LogToFile::~LogToFile() {
			::CloseHandle(m_file);
			delete m_sync;
		}

		LogToFile::LogToFile(PCWSTR path) :
			m_sync(Base::get_LockCritSection()) {
			m_file = ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (m_file) {
				::SetFilePointer(m_file, 0, nullptr, FILE_END);
			}
		}

		void LogToFile::out(const Logger_i * /*lgr*/, Level /*lvl*/, PCWSTR str, size_t size) const {
			DWORD written = 0;
			if (m_file && m_file != INVALID_HANDLE_VALUE) {
				auto lk(m_sync->get_lock());
				::WriteFile(m_file, str, size * sizeof(WCHAR), &written, nullptr);
				//			written /= sizeof(*str);
			}
			//		return written;
		}

		Target_i * get_TargetToFile(PCWSTR path) {
			return new LogToFile(path);
		}

	}

///===================================================================================== Logging
//void logFile(WIN32_FIND_DATA info) {
//	uint64_t size = HighLow64(info.nFileSizeHigh, info.nFileSizeLow);
//	logDebug(L"%s   found: \"%s\" (Size=%I64i,%s%s%s%s%s%s%s%s%s%s%s)\n", FILE_ATTRIBUTE_DIRECTORY
//	    & info.dwFileAttributes ? L"Dir " : L"File", info.cFileName, size, FILE_ATTRIBUTE_ARCHIVE
//	    & info.dwFileAttributes ? L"ARCHIVE " : L"", FILE_ATTRIBUTE_COMPRESSED
//	    & info.dwFileAttributes ? L"COMPRESSED " : L"", FILE_ATTRIBUTE_ENCRYPTED
//	    & info.dwFileAttributes ? L"ENCRYPTED " : L"", FILE_ATTRIBUTE_HIDDEN
//	    & info.dwFileAttributes ? L"HIDDEN " : L"",
//	         FILE_ATTRIBUTE_NORMAL & info.dwFileAttributes ? L"NORMAL " : L"",
//	         FILE_ATTRIBUTE_OFFLINE & info.dwFileAttributes ? L"OFFLINE " : L"",
//	         FILE_ATTRIBUTE_READONLY & info.dwFileAttributes ? L"READONLY " : L"",
//	         FILE_ATTRIBUTE_REPARSE_POINT & info.dwFileAttributes ? L"REPARSE_POINT " : L"",
//	         FILE_ATTRIBUTE_SPARSE_FILE & info.dwFileAttributes ? L"SPARSE " : L"",
//	         FILE_ATTRIBUTE_SYSTEM & info.dwFileAttributes ? L"SYSTEM " : L"",
//	         FILE_ATTRIBUTE_TEMPORARY & info.dwFileAttributes ? L"TEMP " : L"");
//}
}
