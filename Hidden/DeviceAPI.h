#pragma once

// ========================================
//   Device information

#define DEVICE_NAME             L"\\Device\\HiddenGate"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\HiddenGate"
#define DEVICE_WIN32_NAME       L"\\\\.\\HiddenGate"

// ========================================
//   IOCTL codes

#define HID_IOCTL_SET_DRIVER_STATE               CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 +  0), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_GET_DRIVER_STATE               CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 +  1), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_SET_STEALTH_MODE               CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 +  2), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*#define HID_IOCTL_ADD_HIDDEN_REG_KEY             CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 10), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_HIDDEN_REG_KEY          CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 11), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_HIDDEN_REG_KEYS     CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 12), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_ADD_HIDDEN_REG_VALUE           CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 20), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_HIDDEN_REG_VALUE        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 21), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_HIDDEN_REG_VALUES   CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 22), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_ADD_HIDDEN_FILE                CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 30), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_HIDDEN_FILE             CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 31), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_HIDDEN_FILES        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 32), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_ADD_HIDDEN_DIR                 CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 40), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_HIDDEN_DIR              CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 41), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_HIDDEN_DIRS         CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 42), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_ADD_PROTECTED_EXE_PATH         CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 50), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_ATTACH_PROTECTED_EXE_PID       CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 51), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_PROTECTED_EXE_PATH      CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 52), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_PROTECTED_EXE_PATHS CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 53), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define HID_IOCTL_ADD_EXCLUDED_EXE_PATH          CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 54), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_ATTACH_EXCLUDED_EXE_PID        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 55), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_EXCLUDED_EXE_PATH       CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 56), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_EXCLUDED_EXE_PATHS  CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 57), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)*/

#define HID_IOCTL_ADD_HIDDEN_OBJECT              CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 60), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_HIDDEN_OBJECT           CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 61), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define HID_IOCTL_REMOVE_ALL_HIDDEN_OBJECTS      CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 62), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

enum Hid_ObjectTypes {
	RegKeyObject,
	RegValueObject,
	FsFileObject,
	FsDirObject,
	PsProcObject,
};

#pragma pack(push, 4)

typedef struct _Hid_HideObjectPacket {
	unsigned short objType;
	unsigned short size;
}  Hid_HideObjectPacket, *PHid_HideObjectPacket;

typedef struct _Hid_UnhideObjectPacket {
	unsigned short objType;
	unsigned short reserved;
	unsigned long long id;
}  Hid_UnhideObjectPacket, *PHid_UnhideObjectPacket;

typedef struct _Hid_UnhideAllObjectsPacket {
	unsigned short objType;
	unsigned short reserved;
}  Hid_UnhideAllObjectsPacket, *PHid_UnhideAllObjectsPacket;

typedef struct _Hid_StatusPacket {
	unsigned int status;
	union {
		unsigned long long id;
		unsigned long state;
	} info;
}  Hid_StatusPacket, *PHid_StatusPacket;

#pragma pack(pop)
