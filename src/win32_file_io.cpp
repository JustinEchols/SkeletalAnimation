internal void
Win32FileFree(void *Memory)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

internal debug_file
Win32FileReadEntire(char *FileName)
{
	debug_file Result = {};
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = U64TruncateToU32(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Content)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (BytesRead == FileSize32))
				{
					Result.Size = FileSize32;
				}
				else
				{
					Win32FileFree(Result.Content);
					Result.Content = 0;
				}
			}
			else
			{
			}
		}
		else
		{
		}
		CloseHandle(FileHandle);
	}
	else
	{
	}

	return(Result);
}

internal FILETIME 
Win32FileLastWriteTime(char *FileName)
{
	FILETIME LastWriteTime = {};

	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if(GetFileAttributesExA(FileName, GetFileExInfoStandard, &FileData))
	{
		LastWriteTime = FileData.ftLastWriteTime;
	}

	return(LastWriteTime);
}

internal b32
Win32FileTimeCompare(FILETIME T1, FILETIME T2)
{
	b32 Result = (CompareFileTime(&T1, &T2) != 0);
	return(Result);
}
