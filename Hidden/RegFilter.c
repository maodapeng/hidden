// =========================================================================================
//       Registry filter
// =========================================================================================

#include "RegFilter.h"
#include "ExcludeList.h"
#include "PsMonitor.h"

#define FILTER_ALLOC_TAG 'FRlF'

ExcludeContext g_excludeRegKeyContext;
ExcludeContext g_excludeRegValueContext;

CONST PWCHAR g_excludeRegKeys[] = {
//	L"\\REGISTRY\\MACHINE\\SOFTWARE\\test",
//	L"\\Registry\\MACHINE\\SOFTWARE\\test2",
	NULL
};

CONST PWCHAR g_excludeRegValues[] = {
//	L"\\REGISTRY\\MACHINE\\SOFTWARE\\aaa",
//	L"\\Registry\\MACHINE\\SOFTWARE\\xxx",
//	L"\\Registry\\MACHINE\\SOFTWARE\\aa",
//	L"\\Registry\\MACHINE\\SOFTWARE\\aaa",
//	L"\\Registry\\MACHINE\\SOFTWARE\\aaaa",
//	L"\\Registry\\MACHINE\\SOFTWARE\\zz",
	NULL
};

LARGE_INTEGER g_regCookie = { 0 };

BOOLEAN CheckRegistryKeyInExcludeList(PVOID RootObject, PUNICODE_STRING keyPath)
{
	PCUNICODE_STRING regPath;
	NTSTATUS status;
	BOOLEAN found = FALSE;

	// Check is the registry path matched to exclude list

	if (keyPath->Length > sizeof(WCHAR) && keyPath->Buffer[0] == L'\\')
	{
		// Check absolute path
		//DbgPrint("FsFilter1!" __FUNCTION__ ": absolute %wZ\n", keyPath);
		found = CheckExcludeListRegKey(g_excludeRegKeyContext, keyPath);
	}
	else
	{
		// Check relative path
		enum { LOCAL_BUF_SIZE = 256 };
		WCHAR localBuffer[LOCAL_BUF_SIZE];
		LPWSTR dynBuffer = NULL;
		UNICODE_STRING fullRegPath;
		USHORT totalSize;

		// Obtain root key path

		status = CmCallbackGetKeyObjectID(&g_regCookie, RootObject, NULL, &regPath);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("FsFilter1!" __FUNCTION__ ": Registry name query failed with code:%08x\n", status);
			return FALSE;
		}

		// Concatenate root path + sub key path

		totalSize = regPath->Length + keyPath->Length + sizeof(WCHAR);
		if (totalSize / sizeof(WCHAR) > LOCAL_BUF_SIZE)
		{
			// local buffer too small, we should allocate memory
			dynBuffer = (LPWSTR)ExAllocatePoolWithTag(NonPagedPool, totalSize, FILTER_ALLOC_TAG);
			if (!dynBuffer)
			{
				DbgPrint("FsFilter1!" __FUNCTION__ ": Memory allocation failed with code:%08x\n", status);
				return FALSE;
			}

			memcpy(dynBuffer, regPath->Buffer, regPath->Length);
			fullRegPath.Buffer = dynBuffer;
		}
		else
		{
			// use local buffer
			fullRegPath.Buffer = localBuffer;
		}

		// copy root path + sub key path to new buffer
		memcpy(fullRegPath.Buffer, regPath->Buffer, regPath->Length);
		fullRegPath.Buffer[regPath->Length / sizeof(WCHAR)] = L'\\';
		memcpy(
			(PCHAR)fullRegPath.Buffer + regPath->Length + sizeof(WCHAR),
			keyPath->Buffer,
			keyPath->Length);

		fullRegPath.Length = totalSize;
		fullRegPath.MaximumLength = fullRegPath.Length;

		// Compare to exclude list

		//DbgPrint("FsFilter1!" __FUNCTION__ ": relative %wZ\n", &fullRegPath);
		found = CheckExcludeListRegKey(g_excludeRegKeyContext, &fullRegPath);

		if (dynBuffer)
			ExFreePoolWithTag(dynBuffer, FILTER_ALLOC_TAG);
	}

	return found;
}

NTSTATUS RegPreCreateKey(PVOID context, PREG_PRE_CREATE_KEY_INFORMATION info)
{
	UNREFERENCED_PARAMETER(context);

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	DbgPrint("FsFilter1!" __FUNCTION__ ": RegPreCreateKey(absolute) %wZ\n", info->CompleteName);

	if (CheckExcludeListRegKey(g_excludeRegKeyContext, info->CompleteName))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": found!\n");
		return STATUS_ACCESS_DENIED;
	}

	return STATUS_SUCCESS;
}

NTSTATUS RegPreCreateKeyEx(PVOID context, PREG_CREATE_KEY_INFORMATION info)
{
	UNREFERENCED_PARAMETER(context);

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	if (CheckRegistryKeyInExcludeList(info->RootObject, info->CompleteName))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": found!\n");
		return STATUS_ACCESS_DENIED;
	}

	return STATUS_SUCCESS;
}

NTSTATUS RegPreOpenKey(PVOID context, PREG_PRE_OPEN_KEY_INFORMATION info)
{//TODO: isn't used?
	UNREFERENCED_PARAMETER(context);

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	DbgPrint("FsFilter1!" __FUNCTION__ ": RegPreCreateKey(absolute) %wZ\n", info->CompleteName);

	if (CheckExcludeListRegKey(g_excludeRegKeyContext, info->CompleteName))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": found!\n");
		return STATUS_NOT_FOUND;
	}

	return STATUS_SUCCESS;
}

NTSTATUS RegPreOpenKeyEx(PVOID context, PREG_OPEN_KEY_INFORMATION info)
{
	UNREFERENCED_PARAMETER(context);

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	if (CheckRegistryKeyInExcludeList(info->RootObject, info->CompleteName))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": found!\n");
		return STATUS_NOT_FOUND;
	}

	return STATUS_SUCCESS;
}

BOOLEAN GetNameFromEnumKeyPreInfo(KEY_INFORMATION_CLASS infoClass, PVOID infoBuffer, PUNICODE_STRING keyName)
{
	switch (infoClass)
	{
	case KeyBasicInformation:
		{
			PKEY_BASIC_INFORMATION keyInfo = (PKEY_BASIC_INFORMATION)infoBuffer;
			keyName->Buffer = keyInfo->Name;
			keyName->Length = keyName->MaximumLength = (USHORT)keyInfo->NameLength;
		}
		break;
	case KeyNameInformation:
		{
			PKEY_NAME_INFORMATION keyInfo = (PKEY_NAME_INFORMATION)infoBuffer;
			keyName->Buffer = keyInfo->Name;
			keyName->Length = keyName->MaximumLength = (USHORT)keyInfo->NameLength;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

NTSTATUS RegPostEnumKey(PVOID context, PREG_POST_OPERATION_INFORMATION info)
{
	PREG_ENUMERATE_KEY_INFORMATION preInfo;
	PCUNICODE_STRING regPath;
	UNICODE_STRING keyName;
	UINT32 incIndex;
	NTSTATUS status;

	UNREFERENCED_PARAMETER(context);

	if (!NT_SUCCESS(info->Status))
		return STATUS_SUCCESS;

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	status = CmCallbackGetKeyObjectID(&g_regCookie, info->Object, NULL, &regPath);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": Registry name query failed with code:%08x\n", status);
		return STATUS_SUCCESS;
	}

	preInfo = (PREG_ENUMERATE_KEY_INFORMATION)info->PreInformation;

	if (!GetNameFromEnumKeyPreInfo(preInfo->KeyInformationClass, preInfo->KeyInformation, &keyName))
		return STATUS_SUCCESS;

	incIndex = 0;
	if (CheckExcludeListRegKeyValueName(g_excludeRegKeyContext, (PUNICODE_STRING)regPath, &keyName, &incIndex))
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! found %wZ (inc: %d)\n", regPath, incIndex);//TODO: remove it

	if (incIndex > 0)
	{
		HANDLE Key;
		ULONG resLen, i;
		BOOLEAN infinite = TRUE;

		status = ObOpenObjectByPointer(info->Object, OBJ_KERNEL_HANDLE, NULL, KEY_ALL_ACCESS, *CmKeyObjectType, KernelMode, &Key);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("FsFilter1!" __FUNCTION__ ": ObOpenObjectByPointer() failed with code:%08x\n", status);
			return STATUS_SUCCESS;
		}

		for (i = 0; infinite; i++)
		{
			status = ZwEnumerateKey(Key, preInfo->Index + incIndex, preInfo->KeyInformationClass, preInfo->KeyInformation, preInfo->Length, &resLen);
			if (!NT_SUCCESS(status))
				break;

			if (!GetNameFromEnumKeyPreInfo(preInfo->KeyInformationClass, preInfo->KeyInformation, &keyName))
				break;

			if (!CheckExcludeListRegKeyValueName(g_excludeRegKeyContext, (PUNICODE_STRING)regPath, &keyName, &incIndex))
			{
				*preInfo->ResultLength = resLen;
				break;
			}
		}

		info->ReturnStatus = status;

		ZwClose(Key);
	}

	return STATUS_SUCCESS;
}

BOOLEAN GetNameFromEnumValuePreInfo(KEY_VALUE_INFORMATION_CLASS infoClass, PVOID infoBuffer, PUNICODE_STRING keyName)
{
	switch (infoClass)
	{
	case KeyValueBasicInformation:
		{
			PKEY_VALUE_BASIC_INFORMATION keyInfo = (PKEY_VALUE_BASIC_INFORMATION)infoBuffer;
			keyName->Buffer = keyInfo->Name;
			keyName->Length = keyName->MaximumLength = (USHORT)keyInfo->NameLength;
		}
		break;
	case KeyValueFullInformation:
	case KeyValueFullInformationAlign64:
		{
			PKEY_VALUE_FULL_INFORMATION keyInfo = (PKEY_VALUE_FULL_INFORMATION)infoBuffer;
			keyName->Buffer = keyInfo->Name;
			keyName->Length = keyName->MaximumLength = (USHORT)keyInfo->NameLength;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

NTSTATUS RegPostEnumValue(PVOID context, PREG_POST_OPERATION_INFORMATION info)
{
	PREG_ENUMERATE_KEY_INFORMATION preInfo;
	PCUNICODE_STRING regPath;
	UNICODE_STRING keyName;
	UINT32 incIndex;
	NTSTATUS status;

	UNREFERENCED_PARAMETER(context);

	if (!NT_SUCCESS(info->Status))
		return STATUS_SUCCESS;

	if (IsProcessExcluded(PsGetCurrentProcessId()))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! process excluded %d\n", PsGetCurrentProcessId());
		return STATUS_SUCCESS;
	}

	status = CmCallbackGetKeyObjectID(&g_regCookie, info->Object, NULL, &regPath);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": Registry name query failed with code:%08x\n", status);
		return STATUS_SUCCESS;
	}

	preInfo = (PREG_ENUMERATE_KEY_INFORMATION)info->PreInformation;

	if (!GetNameFromEnumValuePreInfo(preInfo->KeyInformationClass, preInfo->KeyInformation, &keyName))
		return STATUS_SUCCESS;

	incIndex = 0;
	if (CheckExcludeListRegKeyValueName(g_excludeRegValueContext, (PUNICODE_STRING)regPath, &keyName, &incIndex))
		DbgPrint("FsFilter1!" __FUNCTION__ ": !!!!! found %wZ (inc: %d)\n", regPath, incIndex);

	if (incIndex > 0)
	{
		HANDLE Key;
		ULONG resLen, i;
		BOOLEAN infinite = TRUE;

		status = ObOpenObjectByPointer(info->Object, OBJ_KERNEL_HANDLE, NULL, KEY_ALL_ACCESS, *CmKeyObjectType, KernelMode, &Key);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("FsFilter1!" __FUNCTION__ ": ObOpenObjectByPointer() failed with code:%08x\n", status);
			return STATUS_SUCCESS;
		}

		for (i = 0; infinite; i++)
		{
			status = ZwEnumerateValueKey(Key, preInfo->Index + incIndex, preInfo->KeyInformationClass, preInfo->KeyInformation, preInfo->Length, &resLen);
			if (!NT_SUCCESS(status))
				break;

			if (!GetNameFromEnumValuePreInfo(preInfo->KeyInformationClass, preInfo->KeyInformation, &keyName))
				break;

			if (!CheckExcludeListRegKeyValueName(g_excludeRegValueContext, (PUNICODE_STRING)regPath, &keyName, &incIndex))
			{
				*preInfo->ResultLength = resLen;
				break;
			}
		}

		info->ReturnStatus = status;

		ZwClose(Key);
	}

	return STATUS_SUCCESS;
}

NTSTATUS RegistryFilterCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2)
{
	REG_NOTIFY_CLASS notifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;
	NTSTATUS status;

	switch (notifyClass)
	{
	case RegNtPreCreateKey:
		status = RegPreCreateKey(CallbackContext, (PREG_PRE_CREATE_KEY_INFORMATION)Argument2);
		break;
	case RegNtPreCreateKeyEx:
		status = RegPreCreateKeyEx(CallbackContext, (PREG_CREATE_KEY_INFORMATION)Argument2);
		break;
	case RegNtPreOpenKey:
		status = RegPreCreateKey(CallbackContext, (PREG_PRE_OPEN_KEY_INFORMATION)Argument2);
		break;
	case RegNtPreOpenKeyEx:
		status = RegPreOpenKeyEx(CallbackContext, (PREG_OPEN_KEY_INFORMATION)Argument2);
		break;
	case RegNtPostEnumerateKey:
		status = RegPostEnumKey(CallbackContext, (PREG_POST_OPERATION_INFORMATION)Argument2);
		break;
	case RegNtPostEnumerateValueKey:
		status = RegPostEnumValue(CallbackContext, (PREG_POST_OPERATION_INFORMATION)Argument2);
		break;
	default:
		status = STATUS_SUCCESS;
		break;
	}

	return status;
}

NTSTATUS InitializeRegistryFilter(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS status;
	UNICODE_STRING altitude, str;
	ExcludeEntryId id;
	UINT32 i;

	// Fill exclude lists

	status = InitializeExcludeListContext(&g_excludeRegKeyContext, ExcludeRegKey);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": exclude registry key list initialization failed with code:%08x\n", status);
		return status;
	}

	for (i = 0; g_excludeRegKeys[i]; i++)
	{
		RtlInitUnicodeString(&str, g_excludeRegKeys[i]);
		AddExcludeListRegistryKey(g_excludeRegKeyContext, &str, &id);
	}

	status = InitializeExcludeListContext(&g_excludeRegValueContext, ExcludeRegValue);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": exclude registry value list initialization failed with code:%08x\n", status);
		DestroyExcludeListContext(g_excludeRegKeyContext);
		return status;
	}

	for (i = 0; g_excludeRegValues[i]; i++)
	{
		RtlInitUnicodeString(&str, g_excludeRegValues[i]);
		AddExcludeListRegistryValue(g_excludeRegValueContext, &str, &id);
	}

	// Register registry filter

	RtlInitUnicodeString(&altitude, L"320000");

	status = CmRegisterCallbackEx(&RegistryFilterCallback, &altitude, DriverObject, NULL, &g_regCookie, NULL);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FsFilter1!" __FUNCTION__ ": Registry filter registration failed with code:%08x\n", status);
		DestroyExcludeListContext(g_excludeRegKeyContext);
		DestroyExcludeListContext(g_excludeRegValueContext);
		return status;
	}

	return status;
}

NTSTATUS DestroyRegistryFilter()
{
	NTSTATUS status;

	status = CmUnRegisterCallback(g_regCookie);
	if (!NT_SUCCESS(status))
		DbgPrint("FsFilter1!" __FUNCTION__ ": Registry filter unregistration failed with code:%08x\n", status);

	return status;
}

NTSTATUS AddHiddenRegKey(PUNICODE_STRING KeyPath, PULONGLONG ObjId)
{
	NTSTATUS status;

	// TODO: normalize registry key path

	status = AddExcludeListRegistryKey(g_excludeRegKeyContext, KeyPath, ObjId);
	
	return status;
}

NTSTATUS RemoveHiddenRegKey(ULONGLONG ObjId)
{
	return RemoveExcludeListEntry(g_excludeRegKeyContext, ObjId);
}

NTSTATUS RemoveAllHiddenRegKeys()
{
	return RemoveAllExcludeListEntries(g_excludeRegKeyContext);
}

NTSTATUS AddHiddenRegValue(PUNICODE_STRING ValuePath, PULONGLONG ObjId)
{
	NTSTATUS status;

	// TODO: normalize registry value path

	status = AddExcludeListRegistryValue(g_excludeRegValueContext, ValuePath, ObjId);

	return status;
}

NTSTATUS RemoveHiddenRegValue(ULONGLONG ObjId)
{
	return RemoveExcludeListEntry(g_excludeRegValueContext, ObjId);
}

NTSTATUS RemoveAllHiddenRegValues()
{
	return RemoveAllExcludeListEntries(g_excludeRegValueContext);
}
