DEBUG_PLATFORM_FILE_FREE(DebugPlatformFileFree)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_FILE_READ_ENTIRE(DebugPlatformFileReadEntire)
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
					DebugPlatformFileFree(Result.Content);
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

DEBUG_PLATFORM_FILE_WRITE_ENTIRE(DebugPlatformFileWriteEntire)
{
	b32 Result = false;
	HANDLE FileHandle = CreateFileA(FileName,
									(GENERIC_READ | GENERIC_WRITE),
									0,
									0,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									0);

	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		Result = WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0);
		if(!Result)
		{
			Assert(0);
		}
		else
		{
			Result = (BytesWritten == MemorySize);
		}

		CloseHandle(FileHandle);
	}

	return(Result);
}

DEBUG_PLATFORM_FILE_GROUP_LOAD(DebugPlatformFileGroupLoad)
{
	file_group_info Result = {};
	MemoryZero(Result.FileNames, sizeof(Result.FileNames));

	WIN32_FIND_DATA Found;
	HANDLE FileHandle = FindFirstFile(DirectoryNameAndWildCard, &Found);
	while(FileHandle != INVALID_HANDLE_VALUE)
	{
		char *Src = Found.cFileName;
		MemoryCopy(StringLen(Src), Src, Result.FileNames[Result.Count++]);
		if(!FindNextFile(FileHandle, &Found)) break;
	}

	return(Result);
}

internal FILETIME 
Win32FileLastWriteTime(char *FileName)
{
	FILETIME LastWriteTime = {};

	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if(GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileData))
	{
		LastWriteTime = FileData.ftLastWriteTime;
	}

	return(LastWriteTime);
}

DEBUG_PLATFORM_FILE_IS_DIRTY(DebugPlatformFileIsDirty)
{
	b32 Result = false;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if(GetFileAttributesEx(Path, GetFileExInfoStandard, &Data))
	{
		u64 NewDate = (((u64)Data.ftLastWriteTime.dwHighDateTime << (u64)32) | (u64)Data.ftLastWriteTime.dwLowDateTime);
		if(NewDate > (*Date))
		{
			*Date = NewDate;
			Result = true;
		}
	}

	return(Result);
}



inline b32
Win32FileHasUpdated(char *FileName, FILETIME LastWriteTime)
{
	FILETIME CurrentWriteTime = Win32FileLastWriteTime(FileName);
	b32 Result = (CompareFileTime(&CurrentWriteTime, &LastWriteTime) != 0);
	return(Result);
}
