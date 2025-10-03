#pragma once

#include <stdint.h>
#include <Windows.h>

#include "FunctionLoaderMacro.h"

class FunctionLoader
{
public:
	FunctionLoader(const char* funcName, const char* dllName);
	~FunctionLoader();

	void CheckNewDLL();
	
	void* GetPtr() const;

private:
	const char* m_FuncName;
	const char* m_DllName;

	uint64_t m_DllHash;
	char m_DllTargetName[19];

	HMODULE m_DllHandle;
	void* m_FuncPtr;

	void LoadNewDLL(uint64_t hash);
	static uint64_t Hash(uint64_t n);
};

