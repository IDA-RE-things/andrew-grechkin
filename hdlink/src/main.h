﻿/**
	hdlink console tool
	Search duplicate files and make hardlinks

	© 2010 Andrew Grechkin

	Some code was adopted from:
	DFHL - Duplicate File Hard Linker, a small tool to gather some space
	from duplicate files on your hard disk
	Copyright (C) 2004, 2005 Jens Scheffler & Oliver Schneider
	http://www.jensscheffler.de/dfhl

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#ifndef __HARDLINKER__HPP
#define __HARDLINKER__HPP

#include <libwin_def/win_def.h>

#include <wincrypt.h>

const		size_t	FirstBlock = 65535;
void		logError(DWORD errNumber, PCWSTR message, ...);

///====================================================================================== Statistics
struct		Statistics {
	size_t		FoundDirs;
	size_t		FoundJuncs;
	size_t		FoundFiles;
	size_t		IgnoredJunc;

	size_t		filesFoundUnique;
	size_t		IgnoredHidden;
	size_t		IgnoredSystem;
	size_t		IgnoredSmall;
	size_t		IgnoredZero;
	size_t		filesOnDifferentVolumes;

	uintmax_t	bytesRead;
	uintmax_t	FoundFilesSize;
	uintmax_t	FreeSpaceIncrease;

	size_t		fileCompares;
	size_t		hashCompares;
	size_t		hashComparesHit1;
	size_t		hashComparesHit2;
	size_t		hashComparesHit3;
	size_t		fileMetaDataMismatch;
	size_t		fileAlreadyLinked;
	size_t		fileContentDifferFirstBlock;
	size_t		fileContentDifferLater;
	size_t		fileContentSame;
	size_t		fileCompareProblems;
	size_t		hashesCalculated;
	size_t		filesOpened;
	size_t		filesClosed;
	size_t		fileOpenProblems;
	size_t		pathObjCreated;
	size_t		pathObjDestroyed;
	size_t		fileObjCreated;
	size_t		fileObjDestroyed;
	size_t		unbufferedFileStreamObjCreated;
	size_t		unbufferedFileStreamObjDestroyed;
	size_t		fileSystemObjCreated;
	size_t		fileSystemObjDestroyed;
	size_t		hardLinks;
	size_t		hardLinksSuccess;
	size_t		collectionObjCreated;
	size_t		collectionObjDestroyed;
	size_t		itemObjCreated;
	size_t		itemObjDestroyed;
	size_t		referenceCounterObjCreated;
	size_t		referenceCounterObjDestroyed;
	size_t		sizeGroupObjCreated;
	size_t		sizeGroupObjDestroyed;
	size_t		sortedFileCollectionObjCreated;
	size_t		sortedFileCollectionObjDestroyed;
	size_t		duplicateEntryObjCreated;
	size_t		duplicateEntryObjDestroyed;
	size_t		duplicateFileCollectionObjCreated;
	size_t		duplicateFileCollectionObjDestroyed;
	size_t		duplicateFileHardLinkerObjCreated;
	size_t		duplicateFileHardLinkerObjDestroyed;

	static	Statistics*  getInstance() {
		static	Statistics instance;
		return	&instance;
	}
};

///==================================================================================== WinCryptProv
class		WinCryptProv: public WinErrorCheck {
	HCRYPTPROV	m_hnd;

public:
	~WinCryptProv() {
		Close();
	}
	// type = (PROV_RSA_FULL, PROV_RSA_AES)
	WinCryptProv(PCWSTR prov = nullptr, DWORD type = PROV_RSA_FULL): m_hnd(0) {
		if (!ChkSucc(::CryptAcquireContextW(&m_hnd, L"MY", prov, type, 0))) {
			ChkSucc(::CryptAcquireContextW(&m_hnd, L"MY", prov, type, CRYPT_NEWKEYSET));
		}
	}
	void			Close() {
		if (m_hnd) {
			::CryptReleaseContext(m_hnd, 0);
			m_hnd = 0;
		}
	}
	operator		HCRYPTPROV() const {
		return	m_hnd;
	}
};

///==================================================================================== WinCryptHash
class		WinCryptHash: public WinErrorCheck {
	HCRYPTHASH		m_handle;

	bool			Close() {
		if (m_handle) {
			::CryptDestroyHash(m_handle);
			return	true;
		}
		return	false;
	}
public:
	~WinCryptHash() {
		Close();
	}
	// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
	WinCryptHash(HCRYPTPROV prov, ALG_ID alg): m_handle(0) {
		ChkSucc(::CryptCreateHash(prov, alg, 0, 0, &m_handle));
	}
	operator		HCRYPTHASH() const {
		return	m_handle;
	}
	bool			Hash(const PBYTE buf, size_t size) {
		return	ChkSucc(::CryptHashData(m_handle, buf, size, 0));
	}
	bool			Hash(PCWSTR path, uint64_t size = (uint64_t) - 1) {
		FileMap	file(path, size);
		if (file.IsOK())
			return	Hash(file);
		err(file.err());
		return	false;
	}
	bool			Hash(FileMap &file) {
		bool	Result = false;
		file.Home();
		while (file.Next())
			Result = Hash((const PBYTE)file.data(), file.size());
		return	Result;
	}
	size_t			GetHashSize() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_HASHVAL, nullptr, &Result, 0);
		return	Result;
	}
	ALG_ID			GetHashAlg() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_ALGID, nullptr, &Result, 0);
		return	Result;
	}
	bool			GetHash(PBYTE buf, DWORD size) const {
		return	ChkSucc(::CryptGetHashParam(m_handle, HP_HASHVAL, buf, &size, 0));
	}
};

///============================================================================================ Path
class		Path {
	Shared_ptr<Path>	parent;
	ustring				name;
public:
	~Path() {
		Statistics::getInstance()->pathObjDestroyed++;
	}
	Path(Shared_ptr<Path> newParent, PCWSTR newName): parent(newParent), name(newName) {
		Statistics::getInstance()->pathObjCreated++;
	}

	void		copyName(PWSTR buf) const {
		if (parent) {
			parent->copyName(buf);
			Cat(buf, PATH_SEPARATOR);
			Cat(buf, name.c_str());
		} else {
			Copy(buf, name.c_str());
		}
	}
	bool		equals(Path* otherPath)  const {
		if (!otherPath || !Eqi(name.c_str(), otherPath->name.c_str())) {
			return	false;
		}
		if (parent != 0) {
			return	parent->equals(otherPath->parent);
		}
		return	true;
	}
};

///======================================================================================== FileHash
class		FileHash {
	static	const size_t	HASH_SIZE = 32;
	BYTE	m_hash[HASH_SIZE];
	bool mutable m_avail;
public:
	FileHash(): m_avail(false) {
		WinMem::Zero(m_hash, size());
	}
	bool		Calculate(PCWSTR path, uint64_t fsize = (uint64_t) - 1) const {
		static WinCryptProv	hCryptProv(nullptr, PROV_RSA_AES);
		DWORD	err = hCryptProv.err();
		if (hCryptProv.IsOK()) {
			WinCryptHash	hSHA(hCryptProv, CALG_SHA1);
			if (hSHA.Hash(path, fsize)) {
				Statistics::getInstance()->hashesCalculated++;
				return	avail(hSHA.GetHash(hash(), size()));
			}
			err = hSHA.err();
			logError(L"%s\n", path);
		}
		logError(L"Unable to count hash: %s\n", ErrAsStr(err).c_str());
		return	false;
	}

	PBYTE	hash(const PBYTE hash, size_t sz) const {
		WinMem::Copy((PVOID)m_hash, (PCVOID)hash, std::min(size(), sz));
		return	(PBYTE)&m_hash;
	}
	PBYTE	hash() const {
		return	(PBYTE)&m_hash;
	}
	size_t	size() const {
		return	HASH_SIZE;
	}
	bool	avail(bool in) const {
		return	m_avail = in;
	}
	bool	avail() const {
		return	m_avail;
	}
	bool	operator==(const FileHash &rhs) const {
		return	WinMem::Cmp(m_hash, rhs.m_hash, size());
	}
	bool	operator!=(const FileHash &rhs) const {
		return	!operator==(rhs);
	}
};

///============================================================================================ File
class		File {
	Shared_ptr<Path>	parent;
	FileHash	m_hashMini;
	FileHash	m_hashFull;
	WinFileId	m_inode;
	ustring		m_name;
	uint64_t	m_size;
	uint64_t	m_time;
	DWORD		m_attr;

public:
	~File() {
		++Statistics::getInstance()->fileObjDestroyed;
	}
	File(Shared_ptr<Path> newParent, const WIN32_FIND_DATAW &info): parent(newParent),
			m_name(info.cFileName),
			m_size(MyUI64(info.nFileSizeLow, info.nFileSizeHigh)),
			m_time(MyUI64(info.ftLastWriteTime.dwLowDateTime, info.ftLastWriteTime.dwHighDateTime)),
			m_attr(info.dwFileAttributes) {
		Statistics::getInstance()->fileObjCreated++;
	}

	ustring			name() const {
		return	m_name;
	}
	DWORD			attr() const {
		return	m_attr;
	}
	uint64_t		time() const {
		return	m_time;
	}
	const FileHash&	hashFull() const {
		return	m_hashFull;
	}
	uint64_t		size() const {
		return	m_size;
	}
	const WinFileId&	inode() const {
		return	m_inode;
	}
	bool			operator<(const File &rhs) const {
		return	m_size < rhs.m_size;
	}
	bool			LoadHashMini() const {
		if (!m_hashMini.avail()) {
			WinBuf<WCHAR>	buf(MAX_PATH_LEN);
			copyName(buf);
			return	m_hashMini.Calculate(buf, FirstBlock);
		}
		return	true;
	}
	bool			LoadHashFull() const {
		if (!m_hashFull.avail()) {
			WinBuf<WCHAR>	buf(MAX_PATH_LEN);
			copyName(buf);
			return	m_hashFull.Calculate(buf);
			/*
						static WinCryptProv	hCryptProv(nullptr, PROV_RSA_AES);
						DWORD	err = hCryptProv.err();
						if (hCryptProv.IsOK()) {
							WinCryptHash	hSHA(hCryptProv, CALG_MD5);
							if (hSHA.Hash(buf)) {
								Statistics::getInstance()->hashesCalculated++;
								m_hash.avail(hSHA.GetHash(m_hash.hash(), m_hash.size()));
								return	m_hash.avail();
							}
							err = hSHA.err();
							logError(L"%s\n", buf.data());
						}
						logError(L"Unable to count hash: %s\n", ErrAsStr(err).c_str());
						return	false;
			*/
		}
		return	true;
	}

	bool			LoadInode() {
		if (m_inode.IsOK())
			return	true;
		WinBuf<WCHAR>	buf(MAX_PATH_LEN);
		copyName(buf);
		bool	Result = m_inode.Load(buf);
		if (!Result)
			logError(L"Unable to load file inode info: %s\n", ErrAsStr().c_str());
		return	Result;
	}

	bool			hardlink(const Shared_ptr<File> &rhs) const {
		++Statistics::getInstance()->hardLinks;
		WinBuf<WCHAR>	file1Name(MAX_PATH_LEN);
		WinBuf<WCHAR>	file2Name(MAX_PATH_LEN);

		this->copyName(file1Name);
		rhs->copyName(file2Name);


		// Step 1: create hard link
		ustring	file2hdlink(file2Name);
		file2hdlink += L".hdlink";
		if (!create_hardlink(file1Name, file2hdlink.c_str())) {
			logError(L"  Unable to create hard link: %i\n", ::GetLastError());
			return	false;
		}

		// Step 2: move file to backup
		ustring	file2backup(file2Name);
		file2backup += L".hdlink-backup";
		if (!move_file(file2Name, file2backup.c_str())) {
			delete_file(file2hdlink);
			logError(L"  Unable to backup file: %i\n", ::GetLastError());
			return	false;
		}

		// Step 3: rename file
		if (!move_file(file2hdlink.c_str(), file2Name)) {
			move_file(file2backup.c_str(), file2Name);
			delete_file(file2hdlink);
			logError(L"  Unable to move file to backup: %i\n", ::GetLastError());
			return	false;
		}

		if (file_exists(file2Name)) {
			delete_file(file2backup) || delete_on_reboot(file2backup);
		}

		{
			ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			logVerbose(L"  Linked!\n");
		}
		Statistics::getInstance()->hardLinksSuccess++;
		return	true;
	}
	void			copyName(PWSTR buf) const {
		parent->copyName(buf);
		Cat(buf, PATH_SEPARATOR);
		Cat(buf, m_name.c_str());
	}
	bool			equals(File* otherFile) const {
		if (!otherFile || !Eqi(m_name.c_str(), otherFile->m_name.c_str())) {
			return	false;
		}
		return	parent->equals(otherFile->parent);
	}

	friend bool		isIdentical(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs);
};

bool		operator==(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	if (lhs->size() != rhs->size())
		return	false;
	if (lhs->inode() == rhs->inode())
		return	true;
	return	false;
}
bool		isIdentical(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	if (lhs->size() > FirstBlock) {
		if (lhs->LoadHashMini() && rhs->LoadHashMini()) {
			if (lhs->m_hashMini != rhs->m_hashMini) {
				++Statistics::getInstance()->fileContentDifferFirstBlock;
				return	false;
			}
		} else {
			return	false;
		}
	}
	if (!lhs->LoadHashFull() || !rhs->LoadHashFull())
		return	false;
	++Statistics::getInstance()->hashCompares;
	return	lhs->m_hashFull == rhs->m_hashFull;
}
bool		isSameVolume(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	return	lhs->inode().vol_sn() == rhs->inode().vol_sn();
}

bool		CompareBySize(const Shared_ptr<File> &f1, const Shared_ptr<File> &f2) {
	if (f1->size() < f2->size())
		return	true;
	return	false;
}
bool		CompareBySizeAndTime(const Shared_ptr<File> &f1, const Shared_ptr<File> &f2) {
	if (f1->size() < f2->size())
		return	true;
	if (f1->size() == f2->size())
		return	f1->time() < f2->time();
	return	false;
}

#endif
