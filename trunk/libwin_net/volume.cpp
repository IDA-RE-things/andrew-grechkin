#include "win_net.h"

#include "volume.h"

void EnumVolumes(ENUMVOLUMECALLBACK callback, PVOID callbackData){
	auto_array<WCHAR> buf(MAX_VOL_LEN);
	HANDLE hndl = CheckHandle(::FindFirstVolumeW(buf, buf.size()));
	while (callback(buf, callbackData) && ::FindNextVolumeW(hndl, buf, buf.size()))
		;
	::FindVolumeClose(hndl);
}

void EnumVolumeMountPoints(ENUMVOLUMECALLBACK callback, PCWSTR volName, PVOID callbackData){
	auto_array<WCHAR> buf(MAX_MOUNT_POINT_LEN);
	HANDLE hndl = CheckHandle(::FindFirstVolumeMountPointW(volName, buf, buf.size()));
	while (callback(buf, callbackData) && ::FindNextVolumeMountPoint(hndl, buf, buf.size()));
	::FindVolumeMountPointClose(hndl);
}

bool GetVolumePathNamesList(PCWSTR volName, PWSTR &mntPointsList, DWORD &listSize){
	wchar_t *_volName=nullptr;//Имя тома
	wchar_t *_mntPointsList=nullptr;//Буфер для точки монтирования
	bool res=true;
	_volName=const_cast<wchar_t*>(volName);
	if (listSize)
		_mntPointsList=mntPointsList;
	if (!listSize){
		GetVolumePathNamesForVolumeNameW(_volName, nullptr, 0, &listSize);
		res=((GetLastError()==ERROR_MORE_DATA) && listSize);
		if (res){
			_mntPointsList=new wchar_t[listSize];
			mntPointsList=_mntPointsList;
		}
	}
	DWORD tmpSize;
	res=res && (GetVolumePathNamesForVolumeNameW(_volName, _mntPointsList, listSize, &tmpSize) || (GetLastError()==ERROR_MORE_DATA));
	mntPointsList=_mntPointsList;
	return res;
}

bool CheckDriveRemovable(PCWSTR name, bool &isRemovable){
	switch (GetDriveType(name)){
		case DRIVE_REMOVABLE:
		case DRIVE_CDROM:
			isRemovable=true;
			return true;
		case DRIVE_FIXED:
		case DRIVE_REMOTE:
		case DRIVE_RAMDISK:
			isRemovable=false;
			return true;
		case DRIVE_UNKNOWN:
		case DRIVE_NO_ROOT_DIR:
		default:
			return false;
 	}
}

AutoUTF GetVolumeByFileName(PCWSTR fileName) {
	auto_array<WCHAR> path(MAX_MOUNT_POINT_LEN);
	CheckApi(::GetVolumePathNameW(fileName, path, path.size()));
	auto_array<WCHAR> name(MAX_VOL_LEN);
	CheckApi(::GetVolumeNameForVolumeMountPointW(path, name, MAX_VOL_LEN));
	return AutoUTF(name);
}

void GetDriveGeometry(PCWSTR name, DISK_GEOMETRY &g) {
	auto_close<HANDLE> hndl(CheckHandle(::CreateFileW(name, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)));
	DWORD size;
	CheckApi(::DeviceIoControl(hndl, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &g, sizeof(g), &size, nullptr));
}

void GetVolumeDiskExtents(PCWSTR name, auto_buf<PVOLUME_DISK_EXTENTS> &buf){
	auto_close<HANDLE> hndl(CheckHandle(::CreateFileW(name, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)));
	DWORD outSize = 0;
	buf.reserve(sizeof(VOLUME_DISK_EXTENTS));
	bool res = ::DeviceIoControl(hndl, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, buf, buf.size(), &outSize, nullptr);
	if (!res && ::GetLastError() == ERROR_MORE_DATA) {
		buf.reserve(buf.size() + (buf->NumberOfDiskExtents - 1) * sizeof(DISK_EXTENT));
		res = ::DeviceIoControl(hndl, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, buf, buf.size(), &outSize, nullptr);
	}
	CheckApi(res);
}