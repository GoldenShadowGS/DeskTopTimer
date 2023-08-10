#pragma once
#include "PCH.h"

class ResourceLoader
{
public:
	ResourceLoader(int Resource, LPCWSTR lpType);
	void Read(void* Buffer, size_t numberBytes);
	void Seek(size_t position);
private:
	size_t SeekPointer = 0;
	BYTE* pData = nullptr;
	size_t DataSize = 0;
};
