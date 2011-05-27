﻿/**
	win_net

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include <winsock2.h>

#include <libwin_def/win_def.h>

//#include <tr1/memory>
//using std::tr1::shared_ptr;

#include <vector>

#include "exception.h"
#include "c_map.h"

#include <sys/types.h>
#include <aclapi.h>
#include <ntsecapi.h>

extern "C" {
	int __cdecl snprintf(char* s, size_t n, const char*  format, ...);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
///========================================================================================== WinNet
namespace	WinNet {
	AutoUTF 	GetCompName(COMPUTER_NAME_FORMAT cnf = ComputerNameNetBIOS);

	inline bool SetCompName(const AutoUTF &in, COMPUTER_NAME_FORMAT cnf) {
		return	::SetComputerNameExW(cnf, in.c_str()) != 0;
	}
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_auth
struct _CREDENTIALW;
const DWORD my_CRED_TYPE_GENERIC = 1;

class Credential {
public:
	~Credential();

	Credential(PCWSTR name, DWORD type = my_CRED_TYPE_GENERIC);

	const _CREDENTIALW* operator->() const;

public:
	static void add(PCWSTR name, PCWSTR pass, PCWSTR target = nullptr);

	static void del(PCWSTR name, DWORD type = my_CRED_TYPE_GENERIC);

private:
	_CREDENTIALW* m_cred;
};

class Credentials {
public:
	~Credentials();

	Credentials();

	void Update();

	size_t size() const;

	const _CREDENTIALW* operator[](size_t ind) const;

private:
	_CREDENTIALW** m_creds;
	DWORD m_size;
};

void	PassSave(PCWSTR name, PCWSTR pass);
inline void	PassSave(const AutoUTF &name, const AutoUTF &pass) {
	PassSave(name.c_str(), pass.c_str());
}

void	PassDel(PCWSTR name);
inline void	PassDel(const AutoUTF &name) {
	PassDel(name.c_str());
}

AutoUTF	PassRead(PCWSTR name);
inline AutoUTF	PassRead(const AutoUTF &name) {
	return 	PassRead(name.c_str());
}

void	PassList();

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_exec
///============================================================================================ Exec
namespace	Exec {
	extern DWORD	TIMEOUT;
	extern DWORD	TIMEOUT_DX;
	void	Run(const AutoUTF &cmd);
	int		Run(const AutoUTF &cmd, astring &out);
	int		Run(const AutoUTF &cmd, astring &out, const astring &in);
	int		RunWait(const AutoUTF &cmd, DWORD wait = TIMEOUT);
	void	RunAsUser(const AutoUTF &cmd, HANDLE token);
	void	RunAsUser(const AutoUTF &cmd, const AutoUTF &user, const AutoUTF &pass);
	int		RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, const AutoUTF &user, const AutoUTF &pass);
	HANDLE	Logon(const AutoUTF &name, const AutoUTF &pass, DWORD type, const AutoUTF &dom = AutoUTF());
	void	Impersonate(HANDLE hToken);
	HANDLE	Impersonate(const AutoUTF &name, const AutoUTF &pass, DWORD type = LOGON32_LOGON_BATCH, const AutoUTF &dom = AutoUTF());
}

///========================================================================================== WinJob
struct		WinJob {
	~WinJob();
	WinJob();
	WinJob(const AutoUTF &name);
	void	SetTimeLimit(size_t seconds);
	void	SetUiLimit();
	void	AddProcess(HANDLE hProc);
	void	RunAsUser(const AutoUTF &cmd, HANDLE hToken);
	int		RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, HANDLE hToken);
private:
	static DWORD	TIMEOUT_DX;
	HANDLE m_job;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_rc
///================================================================================ RemoteConnection
struct		RemoteConnection {
	~RemoteConnection() {
		Close();
	}

	RemoteConnection(PCWSTR host = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr): m_conn(false) {
		Open(host, user, pass);
	}

	void		Open(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	void		Close();

	PCWSTR		host() const {
		return	m_host.c_str();
	}

private:
	AutoUTF	m_host;
	bool	m_conn;
};

///================================================================================== DinamicLibrary
class DynamicLibrary: private Uncopyable {
public:
	~DynamicLibrary() {
		::FreeLibrary(m_hnd);
	}
	DynamicLibrary(PCWSTR path) :
		m_hnd(CheckHandleErr(::LoadLibraryW(path))) {
	}

	operator HMODULE() const {
		return m_hnd;
	}

	HMODULE handle() const {
		return m_hnd;
	}

	FARPROC get_function_nothrow(PCSTR name) const {
		return ::GetProcAddress(m_hnd, name);
	}

	FARPROC get_function(PCSTR name) const {
		return CheckPointer(::GetProcAddress(m_hnd, name));
	}

//	static uint64_t get_version(PCWSTR path) {
//		DWORD handle;
//		DWORD size = ::GetFileVersionInfoSizeW(path, &handle);
//		if (size) {
//			BYTE buf[size];
//			if (::GetFileVersionInfoW(path, handle, size, buf)) {
//				VS_FIXEDFILEINFO* fixed_file_info;
//				UINT len;
//				if (::VerQueryValueW(buf, PATH_SEPARATOR, (PVOID*)&fixed_file_info, &len)) {
//					return HighLow64(fixed_file_info->dwFileVersionMS, fixed_file_info->dwFileVersionLS);
//				}
//			}
//		}
//		return 0;
//	}

private:
	HMODULE m_hnd;
//	AutoUTF m_path;
//	uint64_t m_ver;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sid
///============================================================================================= Sid
/// Security Identifier (Идентификатор безопасности) -
/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
/// домена или компьютера
#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE)((PSID)(&((pACE)->SidStart)))
#endif

class Sid {
	typedef Sid class_type;
public:
	typedef PSID value_type;
	typedef size_t size_type;

	~Sid();

	explicit Sid(WELL_KNOWN_SID_TYPE wns);

	explicit Sid(value_type rhs);

	Sid(const class_type &rhs);

	explicit Sid(PCWSTR name, PCWSTR srv = nullptr);

	explicit Sid(const AutoUTF &name, PCWSTR srv = nullptr);

	class_type& operator=(value_type rhs);

	class_type& operator=(const class_type &rhs);

	bool operator==(value_type rhs) const;

	bool operator==(const class_type &rhs) const {
		return operator==(rhs.m_sid);
	}

	bool operator!=(value_type rhs) const {
		return !operator==(rhs);
	}

	bool operator!=(const class_type &rhs) const {
		return !operator==(rhs.m_sid);
	}

	size_type size() const {
		return class_type::size(m_sid);
	}

	bool is_valid() const {
		return class_type::is_valid(m_sid);
	}

	AutoUTF str() const {
		return class_type::str(m_sid);
	}

	AutoUTF name() const {
		return class_type::name(m_sid);
	}
	AutoUTF full_name() const {
		return class_type::full_name(m_sid);
	}
	AutoUTF domain() const {
		return class_type::domain(m_sid);
	}

	void copy_to(value_type out, size_t size) const;

	operator value_type() const {
		return m_sid;
	}

	void detach(value_type &sid);

	void swap(class_type &rhs);

	static bool is_valid(value_type in) {
		return in && ::IsValidSid(in);
	}
	static void check(value_type in);
	static size_type size(value_type in);
	static size_type sub_authority_count(value_type in);
	static size_type rid(value_type in);

	// PSID to sid string
	static AutoUTF str(value_type in);

	// name to sid string
	static AutoUTF str(const AutoUTF &name, PCWSTR srv = nullptr);

	// PSID to name
	static void name(value_type pSID, AutoUTF &name, AutoUTF &dom, PCWSTR srv = nullptr);
	static AutoUTF name(value_type pSID, PCWSTR srv = nullptr);
	static AutoUTF full_name(value_type pSID, PCWSTR srv = nullptr);
	static AutoUTF domain(value_type pSID, PCWSTR srv = nullptr);

protected:
	Sid(): m_sid(nullptr) {
	}

	value_type m_sid;

private:
	void init(value_type in);
	void init(PCWSTR name, PCWSTR srv = nullptr);
};

class SidString: public Sid {
public:
	explicit SidString(PCWSTR str) {
		init(str);
	}

	explicit SidString(const AutoUTF &str) {
		init(str.c_str());
	}

private:
	void init(PCWSTR str);
};

bool is_admin();

AutoUTF	get_token_user(HANDLE hToken);

///======================================================================================= WinPolicy
namespace	WinPolicy {
	void		InitLsaString(LSA_UNICODE_STRING &lsaString, const AutoUTF &in);
	LSA_HANDLE	GetPolicyHandle(const AutoUTF &dom = L"");
	NTSTATUS	AccountRightAdd(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");
	NTSTATUS	AccountRightDel(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");

	bool		GetTokenUser(HANDLE	hToken, AutoUTF &name);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_log
///========================================================================================== WinLog
class		WinLog {
	HANDLE	m_hndl;

	WinLog();
public:
	~WinLog() {
		::DeregisterEventSource(m_hndl);
	}
	WinLog(PCWSTR name): m_hndl(::RegisterEventSourceW(nullptr, name)) {
		CheckApi(m_hndl != nullptr);
	}
//	void		Write(DWORD Event, WORD Count, LPCWSTR *Strings) {
//		PSID user = nullptr;
//		HANDLE token;
//		PTOKEN_USER token_user = nullptr;
//		if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
//			token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
//			if (token_user)
//				user = token_user->User.Sid;
//			CloseHandle(token);
//		}
//		ReportEventW(m_hndl, EVENTLOG_ERROR_TYPE, 0, Event, user, Count, 0, Strings, nullptr);
//		free(token_user);
//	}

	static void		Register(PCWSTR name, PCWSTR path = nullptr) {
		WCHAR	fullpath[MAX_PATH_LEN];
		WCHAR	key[MAX_PATH_LEN];
		if (!path || Empty(path)) {
			CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
		} else {
			Copy(fullpath, path, sizeofa(fullpath));
		}
		HKEY	hKey = nullptr;
		Copy(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", sizeofa(key));
		Cat(key, name, sizeofa(key));
		CheckApi(::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey) == ERROR_SUCCESS);
		// Add the Event ID message-file name to the subkey.
		::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((Len(fullpath) + 1)*sizeof(WCHAR)));
		// Set the supported types flags.
		DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
		::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
		::RegCloseKey(hKey);
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_SD
const DWORD ALL_SD_INFO = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

AutoUTF	GetOwner(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF	GetOwner(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

AutoUTF	GetGroup(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF	GetGroup(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetOwner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetOwner(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetOwnerSD(const AutoUTF &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetGroup(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetGroup(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetGroupSD(const AutoUTF &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetDacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetDacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetDacl(const AutoUTF &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetSacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const AutoUTF &path, const AutoUTF &sddl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const AutoUTF &path, const Sid &uid, const Sid &gid, mode_t mode, bool protect = false, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

AutoUTF	Mode2Sddl(const AutoUTF &name, const AutoUTF &group, mode_t mode);
AutoUTF	MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool protect = false);

///=========================================================================================== WinSD
/// Security descriptor (Дескриптор защиты)
/// Version		- версия SD (revision)
/// Flags		- флаги состояния
/// Owner SID	- sid владельца
/// Group SID	- sid группы (не используется вендой, лишь для совместимости с POSIX)
/// DACL		- список записей контроля доступа
/// SACL		- список записей аудита
class	WinSD {
public:
	virtual ~WinSD() = 0;

	operator	PSECURITY_DESCRIPTOR() const {
		return	m_sd;
	}

	bool	IsProtected() const {
		return	is_protected(m_sd);
	}
	bool	IsSelfRelative() const {
		return	is_selfrelative(m_sd);
	}
	DWORD	Size() const {
		return	size(m_sd);
	}

	WORD	Control() const {
		return	get_control(m_sd);
	}
	void	Control(WORD flag, bool s) {
		set_control(m_sd, flag, s);
	}
	AutoUTF	Owner() const {
		return	Sid::name(get_owner(m_sd));
	}
	void	SetOwner(PSID pSid, bool deflt = false) {
		set_owner(m_sd, pSid, deflt);
	}
	AutoUTF	Group() const {
		return	Sid::name(get_group(m_sd));
	}
	void	SetGroup(PSID pSid, bool deflt = false) {
		set_group(m_sd, pSid, deflt);
	}
	PACL	Dacl() const {
		return	get_dacl(m_sd);
	}
	void	SetDacl(PACL dacl) {
		set_dacl(m_sd, dacl);
	}
	void	MakeSelfRelative();
	void	Protect(bool pr) {
		Control(SE_DACL_PROTECTED, pr);
	}

	AutoUTF	as_sddl(SECURITY_INFORMATION in = ALL_SD_INFO) const {
		return	as_sddl(m_sd, in);
	}

	static void Free(PSECURITY_DESCRIPTOR &in);

	static bool is_valid(PSECURITY_DESCRIPTOR sd) {
		return	sd && ::IsValidSecurityDescriptor(sd);
	}
	static bool is_protected(PSECURITY_DESCRIPTOR sd);
	static bool is_selfrelative(PSECURITY_DESCRIPTOR sd);

	static WORD get_control(PSECURITY_DESCRIPTOR sd);
	static size_t size(PSECURITY_DESCRIPTOR sd);

	static PSID get_owner(PSECURITY_DESCRIPTOR sd);
	static PSID get_group(PSECURITY_DESCRIPTOR sd);
	static PACL get_dacl(PSECURITY_DESCRIPTOR sd);

	static void set_control(PSECURITY_DESCRIPTOR sd, WORD flag, bool s);
	static void set_owner(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
	static void set_group(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
	static void set_dacl(PSECURITY_DESCRIPTOR sd, PACL acl);
	static void set_sacl(PSECURITY_DESCRIPTOR sd, PACL acl);

	static AutoUTF	as_sddl(PSECURITY_DESCRIPTOR sd, SECURITY_INFORMATION in = ALL_SD_INFO);
	static AutoUTF	Parse(PSECURITY_DESCRIPTOR sd);

protected:
	PSECURITY_DESCRIPTOR	m_sd;

	WinSD(): m_sd(nullptr) {
	}
};

/// Security descriptor by SDDL
class	WinSDDL: public WinSD {
public:
	WinSDDL(const AutoUTF &in);
};

/// Absolute Security descriptor
class	WinAbsSD: public WinSD {
public:
	~WinAbsSD();
	WinAbsSD();
	WinAbsSD(const WinSD &sd) {
		Init((PSECURITY_DESCRIPTOR)sd);
	}
	WinAbsSD(PSECURITY_DESCRIPTOR sd) {
		Init(sd);
	}
	WinAbsSD(const AutoUTF &name, const AutoUTF &group, bool protect = true);
	WinAbsSD(const AutoUTF &usr, const AutoUTF &grp, mode_t mode, bool protect = true);
	WinAbsSD(PSID ow, PSID gr, PACL dacl, bool protect = true);
private:
	void	Init(PSECURITY_DESCRIPTOR sd);

	PSID	m_owner;
	PSID	m_group;
	PACL	m_dacl;
	PACL	m_sacl;
};

/// Security descriptor by handle
class	WinSDH: public WinSD {
public:
	WinSDH(HANDLE handle, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_hnd(handle), m_type(type) {
		Get();
	}
	HANDLE	hnd() const {
		return	m_hnd;
	}
	void	Get();
	void	Set() const;
private:
	HANDLE			m_hnd;
	SE_OBJECT_TYPE	m_type;
};

/// Security descriptor by name
class	WinSDW: public WinSD {
public:
	WinSDW(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_name(name), m_type(type) {
		Get();
	}
	AutoUTF	name() const {
		return	m_name;
	}
	void	Get();
	void	Set() const;
	void	Set(const AutoUTF &path) const;
private:
	AutoUTF			m_name;
	SE_OBJECT_TYPE	m_type;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_dacl
class Trustee: public TRUSTEEW {
public:
	Trustee(PCWSTR name);

	Trustee(PSID sid);
};

///======================================================================================= ExpAccess
class ExpAccess: public EXPLICIT_ACCESSW {
public:
	ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh = SUB_CONTAINERS_AND_OBJECTS_INHERIT);

	AutoUTF get_name() const;

	AutoUTF get_fullname() const;

	Sid		get_sid() const;
};

class ExpAccessArray {
public:
	~ExpAccessArray();

	ExpAccessArray(PACL acl);

	ExpAccess& operator[](int i) const {
		return m_eacc[i];
	}

	size_t count() const {
		return m_cnt;
	}

private:
	ExpAccess *m_eacc;
	ULONG m_cnt;
};

ACCESS_MASK eff_rights(const PSECURITY_DESCRIPTOR psd, PSID sid);

size_t		access2mode(ACCESS_MASK acc);

ACCESS_MASK mode2access(size_t mode);

///========================================================================================= WinDacl
class WinDacl {
public:
	~WinDacl();
	WinDacl(size_t size);
	WinDacl(PACL acl);
	WinDacl(PSECURITY_DESCRIPTOR sd);
	WinDacl(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	operator	PACL() const {
		return	m_dacl;
	}

	PACL* operator&();

	void Add(const ExpAccess &acc);
	void Set(PCWSTR name, ACCESS_MASK acc);
	void Revoke(PCWSTR name);
	void Grant(PCWSTR name, ACCESS_MASK acc);
	void Deny(PCWSTR name, ACCESS_MASK acc);

	void	SetTo(DWORD flag, const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT) const {
		WinDacl::set(name.c_str(), m_dacl, flag, type);
	}

	size_t	count() const {
		return	count(m_dacl);
	}

	size_t	size() const {
		return	size(m_dacl);
	}

	void	detach(PACL &acl);
	void	swap(WinDacl &rhs);

	static AutoUTF	Parse(PACL acl);

	static bool		is_valid(PACL in) {
		return	::IsValidAcl(in);
	}
	static void		get_info(PACL acl, ACL_SIZE_INFORMATION &out);
	static size_t	count(PACL acl);
	static size_t	used_bytes(PACL acl);
	static size_t	free_bytes(PACL acl);
	static size_t	size(PACL acl);
	static PVOID	get_ace(PACL acl, size_t index);

	static void del_inherited_aces(PACL acl);

	static void set(PCWSTR path, PACL dacl, DWORD flag, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_inherit(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect_copy(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static void inherit(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect_copy(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static PACL create(size_t size);

private:
	WinDacl(): m_dacl(nullptr) {
	}
	void	Init(PACL acl);
	void	Init(PSECURITY_DESCRIPTOR sd);

	PACL	m_dacl;
};

///======================================================================================= WinAccess
struct		AccessInfo {
	AutoUTF	type;
	AutoUTF unix;
	DWORD	mask;
};

class		WinAccess : public MultiMapContainer<AutoUTF, AccessInfo> {
public:
	WinAccess(const WinSD &sd, bool autocache = true): acl(sd.Dacl()) {
		if (autocache)
			Cache();
	}

	bool			Cache();

private:
	PACL acl;
};

///==================================================================================== WinSysTimers
struct		WinSysTimers {
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;

	WinSysTimers();

	size_t	Uptime(size_t del = 1);

	AutoUTF	UptimeAsText();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_if
///=========================================================================================== WinIf
inline AutoUTF		MacAsStr(const PBYTE mac, size_t size) {
	WCHAR	buf[(size + 1) * 4];
	WinMem::Zero(buf, sizeof(buf));
	PWSTR	tmp = buf;
	for (size_t i = 0; i < size; ++i) {
		if (i == (size - 1))
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X", mac[i]);
		else
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X-", mac[i]);
	}
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(LPSOCKADDR addr, size_t	len) {
	WCHAR	buf[64];
	WinMem::Zero(buf, sizeof(buf));
	DWORD	size = sizeofa(buf);
	::WSAAddressToStringW(addr, len, nullptr, buf, &size);
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(SOCKET_ADDRESS pAddr) {
	return	IpAsStr(pAddr.lpSockaddr, pAddr.iSockaddrLength);
}

class	WinIp: public MapContainer<AutoUTF, SOCKET_ADDRESS> {
public:
	WinIp(bool autocache = true) {
		if (autocache)
			Cache();
	}
	bool		Cache();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sock
///======================================================================================== WSockLib
class		WSockLib: private Uncopyable {
public:
	~WSockLib() {
		::WSACleanup();
	}

	static WSockLib &Init() {
		static WSockLib init;
		return	init;
	}

private:
	WSockLib() {
		CheckWSock(::WSAStartup(MAKEWORD(1, 2), &wsaData));
	}

	WSADATA	wsaData;
};

///=========================================================================================== WSock
class		WSock {
public:
	~WSock() {
		::closesocket(m_sock);
	}
	WSock(int fam = AF_INET): m_sock(INVALID_SOCKET), m_fam(fam) {
		WSockLib::Init();
		m_sock = ::socket(m_fam, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
			throw	WSockError("WinSock allocate socket error: ");
	}

	void		Connect(const AutoUTF &ip, DWORD port) {
		INT		size = 128;
		auto_buf<PSOCKADDR>	addr(size);
		INT		err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size);
		if (err && err != WSAEFAULT)
			throw	WSockError("WinSock determine address error: ");
		if (err == WSAEFAULT) {
			addr.reserve(size);
			err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size);
			if (err)
				throw	WSockError("WinSock determine address error: ");
		}
		if (m_fam == AF_INET || m_fam == AF_INET6) {
			sockaddr_in* tmp = (sockaddr_in*)addr.data();
			tmp->sin_port = htons(port);
		}
		if (::connect(m_sock, addr.data(), size))
			throw	WSockError("WinSock connect error: ");

	}

	void		Send(void* buf, size_t len) {
		::send(m_sock, (const char*)buf, len, 0);
	}

	operator	SOCKET() {
		return	m_sock;
	}

private:
	SOCKET	m_sock;
	int		m_fam;
};

#endif // WIN_NET_HPP
