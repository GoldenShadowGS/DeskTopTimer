#include "PCH.h"
#include "ResourceLoader.h"

ResourceLoader::ResourceLoader(int Resource, LPCWSTR lpType)
{
	HRSRC res = FindResource(NULL, MAKEINTRESOURCE(Resource), lpType);
	if (res)
	{
		HGLOBAL res_handle = LoadResource(GetModuleHandleW(NULL), res);
		if (res_handle)
		{
			pData = (BYTE*)LockResource(res_handle);
			DataSize = SizeofResource(GetModuleHandleW(NULL), res);
		}
	}
}

void ResourceLoader::Read(void* Buffer, size_t numberBytes)
{
	memcpy(Buffer, pData + SeekPointer, numberBytes);
	SeekPointer += numberBytes;
}

void ResourceLoader::Seek(size_t position)
{
	SeekPointer = position;
}