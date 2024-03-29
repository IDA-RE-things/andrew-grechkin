﻿/** 7zip.h
 *	@interface to 7-zip
 *	@author © 2011 Andrew Grechkin
 *	@link (ole32 oleaut32 uuid)
 **/

#ifndef WIN_7ZIP_HPP
#define WIN_7ZIP_HPP

#include <libcom/win_com.hpp>
#include <libext/dll.hpp>
#include <libext/fileversion.hpp>
#include <libext/file.hpp>
#include <libbase/std.hpp>

#include <initguid.h>
#include <shobjidl.h>

#include <map>
#include <vector>
#include <tr1/memory>

namespace SevenZip {
	#include <lib7zip/CPP/7zip/Archive/IArchive.h>
	#include <lib7zip/CPP/7zip/IPassword.h>

	class Lib;

	const UInt64 max_check_size = 1024 * 1024;

	enum CompressMethod {
		metCopy	= 0,
		metDelta = 3,
		met7zAES = 116459265,
		metARM = 50529537,
		metARMT	= 50530049,
		metBCJ = 50528515,
		metBCJ2	= 50528539,
		metBZip2 = 262658,
		metDeflate = 262408,
		metDeflate64 = 262409,
		metIA64	= 50529281,
		metLZMA	= 196865,
		metLZMA2 = 33,
		metPPC = 50528773,
		metPPMD	= 197633,
		metRar1	= 262913,
		metRar2	= 262914,
		metRar3	= 262915,
		metSPARC = 50530309,
		metSwap2 = 131842,
		metSwap4 = 131844,
	};

	extern const Base::NamedValues<int> ArcItemPropsNames[63];
	extern const Base::NamedValues<CompressMethod> CompressMethodsNames[21];

	struct FailedFile {
		ustring path;
		HRESULT code;

		FailedFile(const ustring & p, HRESULT h):
			path(p),
			code(h) {
		}
	};

	typedef std::vector<FailedFile> FailedFiles;

	struct DirItem: public Ext::WinFileInfo {
		ustring path;
		ustring name;
		DirItem(const ustring & file_path, const ustring & file_name);
	};

	///====================================================================================== Method
	struct Method {
		uint64_t id;
		ustring name;
		std::vector<BYTE> start_sign, finish_sign;

	private:
		bool operator <(const Method & rhs) const;

		bool operator ==(const Method & rhs) const;

		bool operator !=(const Method & rhs) const;

		Method(const Lib & arc_lib, size_t idx);

		friend class Methods;
	};

	///===================================================================================== Methods
	struct Methods: private std::map<uint64_t, std::tr1::shared_ptr<Method> > {
		typedef map::const_iterator iterator;
		typedef map::const_iterator const_iterator;

		using map::begin;
		using map::end;
		using map::empty;
		using map::size;
		using map::at;

		void cache(const Lib & lib);

	private:
		Methods();
		Methods(const Lib & lib);

		friend class Lib;
	};

	///======================================================================================= Codec
	struct Codec {
		ustring name;
		ustring ext;
		ustring add_ext;
		ustring kAssociate;
		WinGUID guid;
		std::vector<BYTE> start_sign, finish_sign;
		bool updatable;
		bool kKeepName;

//		ustring default_extension() const;

	private:
		bool operator <(const Codec & rhs) const;

		bool operator ==(const Codec & rhs) const;

		bool operator !=(const Codec & rhs) const;

		Codec(const Lib & arc_lib, size_t idx);

		friend class Codecs;
	};

	///====================================================================================== Codecs
	struct Codecs: private std::map<ustring, std::tr1::shared_ptr<Codec> > {
		typedef map::const_iterator iterator;
		typedef map::const_iterator const_iterator;

		using map::begin;
		using map::end;
		using map::empty;
		using map::size;
		using map::at;

		void cache(const Lib & lib);

//		ArcTypes find_by_ext(const ustring & ext) const;

	private:
		Codecs();
		Codecs(const Lib & lib);

		friend class Lib;
	};

	///======================================================================================== Prop
	struct Prop {
		ustring name;
		PROPID id;
		PropVariant prop;

	private:
		Prop(const ComObject<IInArchive> & arc, size_t idx);

		friend class Props;
	};

	///======================================================================================= Props
	struct Props: private std::vector<Prop> {
		typedef vector::const_iterator iterator;
		typedef vector::const_iterator const_iterator;

		using vector::begin;
		using vector::end;
		using vector::empty;
		using vector::size;
		using vector::at;

		void cache(const ComObject<IInArchive> & arc);

	private:
		using vector::push_back;

		Props();
		Props(const ComObject<IInArchive> & arc);

		friend class Archive;
	};

	///========================================================================================= Lib
	class Lib: public Ext::FileVersion, private Ext::DynamicLibrary {
		typedef UInt32 (WINAPI *FCreateObject)(const GUID * clsID, const GUID * interfaceID, PVOID * outObject);
		typedef UInt32 (WINAPI *FGetHandlerProperty)(PROPID propID, PROPVARIANT * value);
		typedef UInt32 (WINAPI *FGetHandlerProperty2)(UInt32 index, PROPID propID, PROPVARIANT * value);
		typedef UInt32 (WINAPI *FGetMethodProperty)(UInt32 index, PROPID propID, PROPVARIANT * value);
		typedef UInt32 (WINAPI *FGetNumberOfMethods)(UInt32 *numMethods);
		typedef UInt32 (WINAPI *FGetNumberOfFormats)(UInt32 * numFormats);
		typedef UInt32 (WINAPI *FSetLargePageMode)();

	public:
		DEFINE_FUNC(CreateObject);
		DEFINE_FUNC(GetHandlerProperty);
		DEFINE_FUNC(GetHandlerProperty2);
		DEFINE_FUNC(GetMethodProperty);
		DEFINE_FUNC(GetNumberOfFormats);
		DEFINE_FUNC(GetNumberOfMethods);
		DEFINE_FUNC(SetLargePageMode);

		Lib(PCWSTR path);

		const Codecs & codecs() const;
		const Methods & methods() const;

		HRESULT get_prop(UInt32 index, PROPID prop_id, PropVariant & prop) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, WinGUID & guid) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, bool & value) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, ustring & value) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, std::vector<BYTE> & value) const;

	private:
		Codecs m_codecs;
		Methods m_methods;
	};

	///=================================================================================== SfxModule
//	struct SfxModule {
//		ustring path;
//		ustring description() const;
//		bool all_codecs() const;
//		bool install_config() const;
//	};
//
//	class SfxModules: public vector<SfxModule> {
//	public:
//		unsigned find_by_name(const ustring & name) const;
//	};

	///============================================================================== FileReadStream
	struct FileReadStream: public IInStream, private Ext::WinFile, private UnknownImp {
		virtual ~FileReadStream();

		FileReadStream(const ustring & path);

		// Unknown
		virtual ULONG WINAPI AddRef();
		virtual ULONG WINAPI Release();
		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** object);

		// ISequentialInStream
		virtual HRESULT WINAPI Read(void * data, UInt32 size, UInt32 * processedSize);

		// IInStream
		virtual HRESULT WINAPI Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
	};

	///============================================================================= FileWriteStream
	struct FileWriteStream: public IOutStream, private Ext::WinFile, private UnknownImp {
		virtual ~FileWriteStream();

		FileWriteStream(const ustring & path, DWORD creat = CREATE_NEW);

		// Unknown
		virtual ULONG WINAPI AddRef();
		virtual ULONG WINAPI Release();
		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** object);

		// ISequentialOutStream
		virtual HRESULT WINAPI Write(PCVOID data, UInt32 size, UInt32 * processedSize);

		// IOutStream
		virtual HRESULT WINAPI Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
		virtual HRESULT WINAPI SetSize(UInt64 newSize);

		using WinFile::set_mtime;
	};

	///================================================================================ OpenCallback
	struct OpenCallback: public IArchiveOpenCallback, public ICryptoGetTextPassword, private UnknownImp {
		ustring Password;

		virtual ~OpenCallback();

		OpenCallback();

		// IUnknown
		virtual ULONG WINAPI AddRef();
		virtual ULONG WINAPI Release();
		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** object);

		// IArchiveOpenCallback
		virtual HRESULT WINAPI SetTotal(const UInt64 * files, const UInt64 * bytes);
		virtual HRESULT WINAPI SetCompleted(const UInt64 * files, const UInt64 * bytes);

		// ICryptoGetTextPassword
		virtual HRESULT WINAPI CryptoGetTextPassword(BSTR * password);
	};

	///===================================================================================== Archive
	class Archive: private Base::Uncopyable {
		class const_input_iterator;

	public:
		typedef Archive this_type;
		typedef size_t size_type;
		typedef int flags_type;
		typedef const_input_iterator iterator;
		typedef const_input_iterator const_iterator;

		enum search_flags {
			incDots = 0x0001,
			skipDirs = 0x0002,
			skipFiles = 0x0004,
			skipLinks = 0x0008,
			skipHidden = 0x0010,
		};

		Archive(const Lib & lib, const ustring & path, flags_type flags = 0);

		Archive(const Lib & lib, const ustring & path, const ustring & mask, flags_type flags = 0);

		Archive(ComObject<IInArchive> arc, flags_type flags = 0);

		const Codec & codec() const;

		ComObject<IInArchive> operator ->() const;

		const_iterator begin() const;

		const_iterator end() const;

		const_iterator at(size_t index) const;

		bool empty() const;

		size_t size() const;

		flags_type flags() const;

		const Props & props() const;

		size_t test() const;

		void extract(const ustring & dest) const;

		operator ComObject<IInArchive>() const;

	private:
		void open_archive(const Lib & lib, const ustring & path);

		void init_props();

		ComObject<IInArchive> m_arc;
		Codecs::const_iterator m_codec;
		Props m_props;
		UInt32 m_size;
		flags_type m_flags;
	};

	struct Archive::const_input_iterator {
		typedef const_input_iterator this_type;

		this_type & operator ++();

		this_type operator ++(int);

		ustring path() const;

		uint64_t size() const;

		size_t attr() const;

		FILETIME mtime() const;

		bool is_file() const;

		bool is_dir() const;

		size_t get_props_count() const;

		bool get_prop_info(size_t index, ustring & name, PROPID & id) const;

		PropVariant get_prop(PROPID id) const;

		bool operator ==(const this_type & rhs) const;

		bool operator !=(const this_type & rhs) const;

	private:
		const_input_iterator();
		const_input_iterator(const Archive & seq);
		const_input_iterator(const Archive & seq, UInt32 index);

		Archive * m_seq;
		UInt32 m_index;
		bool m_end;

		friend class Archive;
	};

	///============================================================================= ExtractCallback
	struct ExtractCallback: public IArchiveExtractCallback, public ICryptoGetTextPassword, private UnknownImp {
		FailedFiles failed_files;
		ustring Password;

		virtual ~ExtractCallback();

		// Unknown
		virtual ULONG WINAPI AddRef();
		virtual ULONG WINAPI Release();
		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** object);

		// IProgress
		virtual HRESULT WINAPI SetTotal(UInt64 size);
		virtual HRESULT WINAPI SetCompleted(const UInt64 * completeValue);

		// IArchiveExtractCallback
		virtual HRESULT WINAPI GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode);
		virtual HRESULT WINAPI PrepareOperation(Int32 askExtractMode);
		virtual HRESULT WINAPI SetOperationResult(Int32 resultEOperationResult);

		// ICryptoGetTextPassword
		virtual HRESULT WINAPI CryptoGetTextPassword(BSTR * pass);

	private:
		ExtractCallback(const Archive & arc, const ustring & dest_path = ustring(), const ustring & pass = ustring());

		const Archive & m_wa;
		ustring m_dest;				// Output directory

		struct CurrItem;
		std::tr1::shared_ptr<CurrItem> m_curr;

		friend class Archive;
	};

	///=========================================================================== ArchiveProperties
	struct CompressProperties: private std::vector<DirItem> {
		typedef vector::const_iterator iterator;

		using vector::begin;
		using vector::end;
		using vector::empty;
		using vector::size;
		using vector::at;

		std::vector<UInt64> VolumesSizes;
		ustring VolName;
		ustring VolExt;
		ustring password;
		size_t level;
		bool solid;
		bool encrypt_header;
		bool AskPassword;
		bool IgnoreErrors;
		bool silent;

		CompressProperties();

		void add(const ustring & add_path);

		void writeln(PCWSTR str) const;

		void writeln(const ustring & str) const;

		void printf(PCWSTR format, ...) const;

	private:
		void base_add(const ustring & base_path, const ustring & name);
	};

	///============================================================================== UpdateCallback
	struct UpdateCallback: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, private UnknownImp {
		virtual ~UpdateCallback();

		UpdateCallback(const CompressProperties & props, FailedFiles & ffiles);

		// Unknown
		virtual ULONG WINAPI AddRef();
		virtual ULONG WINAPI Release();
		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** object);

		// IProgress
		virtual HRESULT WINAPI SetTotal(UInt64 size);
		virtual HRESULT WINAPI SetCompleted(const UInt64 * completeValue);

		// GetUpdateItemInfo
		virtual HRESULT WINAPI GetUpdateItemInfo(UInt32 index, Int32 * newData, Int32 * newProperties, UInt32 * indexInArchive);
		virtual HRESULT WINAPI GetProperty(UInt32 index, PROPID propID, PROPVARIANT * value);
		virtual HRESULT WINAPI GetStream(UInt32 index, ISequentialInStream ** inStream);
		virtual HRESULT WINAPI SetOperationResult(Int32 operationResult);

		// IArchiveUpdateCallback2
		virtual HRESULT WINAPI GetVolumeSize(UInt32 index, UInt64 * size);
		virtual HRESULT WINAPI GetVolumeStream(UInt32 index, ISequentialOutStream ** volumeStream);

		// ICryptoGetTextPassword2
		virtual HRESULT WINAPI CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password);

	private:
		const CompressProperties & m_props;
		FailedFiles & m_ffiles;
	};

	///=============================================================================== CreateArchive
	struct CreateArchive: public CompressProperties, private Base::Uncopyable {
		CreateArchive(const Lib & lib, const ustring & codec);

		void compress(const ustring & path);

//		ComObject<IOutArchive> operator ->() const;

	private:
		void set_properties();

		const Lib & m_lib;
		const ustring m_codec;
		ComObject<IOutArchive> m_arc;
		FailedFiles m_ffiles;
	};
}

#endif
