#include "FunctionLoader.h"

FunctionLoader::FunctionLoader(const char* funcName, const char* dllName)
	: m_FuncName(funcName), m_DllName(dllName), m_DllHash(0ULL), m_FuncPtr(nullptr)
{
	// Write the beginning and the end of the target filepath, which will always be the same

	m_DllTargetName[0] = 'g';
	m_DllTargetName[1] = 'e';
	m_DllTargetName[2] = 'n';
	m_DllTargetName[3] = '\\';

	m_DllTargetName[14] = '.';
	m_DllTargetName[15] = 'd';
	m_DllTargetName[16] = 'l';
	m_DllTargetName[17] = 'l';
	m_DllTargetName[18] = '\0';
}

FunctionLoader::~FunctionLoader()
{
	if (m_FuncPtr)
	{
		FreeLibrary(m_DllHandle);
		DeleteFileA(m_DllTargetName);
	}
}

void* FunctionLoader::GetPtr() const
{
	return m_FuncPtr;
}

void FunctionLoader::CheckNewDLL()
{
	// Get the file data
	WIN32_FILE_ATTRIBUTE_DATA fileData = {};
	if (!GetFileAttributesExA(m_DllName, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData))
		return;

	// Get the time since the creation (last write) of the DLL file
	uint64_t msb = uint64_t(fileData.ftLastWriteTime.dwHighDateTime);
	uint64_t lsb = uint64_t(fileData.ftLastWriteTime.dwLowDateTime);
	uint64_t hash = Hash((msb << 32) | lsb);

	// If the hash of the last write time of the file being checked is different from
	// the current one in use, the DLL is different and therefore must be reloaded
	if (hash != m_DllHash)
		LoadNewDLL(hash);

	/*
		If the hash of the source file and the current file are the same,
		the DLL is almost certainly the exact same too, so don't change anything.

		This approach might not cover all possible scenarios, but it is very efficient,
		and it covers over 99% of them.

		I've not yet been able to produce a scenario in which this function fails to
		load a new DLL file (no false negatives), even though it does reload the same DLL file
		if you recompile it (false positives), which might even be an advantageous feature.

		If you still want to be sure it covers 100% of the cases, you must implement a
		checksum over all the bytes of the input file, and use that as a hash.
	*/
}
	
/*
	// Open source DLL file
	FILE* source = fopen(m_DllName, "rb");
	if (!source) return;

	fseek(source, 0, SEEK_END);
	size_t sourceSize = ftell(source);
	if (sourceSize <= 0)
	{
		fclose(source);
		return;
	}

	// Reset the file pointer to the start of the file
	if (fseek(source, 0, SEEK_SET) != 0)
	{
		fclose(source);
		return;
	}

	// Allocate memory for new buffer
	uint8_t* sourceBuffer = (uint8_t*)malloc(m_DllBufferSize);
	// If fail to allocate, return
	if (!sourceBuffer)
	{
		fclose(source);
		return;
	}

	// Copy the file contents to the buffer
	fread(m_DllBuffer, 1, m_DllBufferSize, source);
	// Close the file
	fclose(source);

	// Initialize variables to compute checksum
	constexpr size_t wordSize = sizeof(uint64_t);
	uint64_t word = 0ULL;
	uint64_t checksum = 0ULL;

	// Hash the contents of the buffer into a checksum
	for (size_t i = 0; i < m_DllBufferSize; i++)
	{
		// Get byte index inside uint64_t
		size_t j = i % wordSize;
		// Accumulate byte into word
		word |= (uint64_t(m_DllBuffer[i]) << (j * wordSize));

		// When the last byte of the word is reached
		if (j == wordSize - 1)
		{
			// Hash the last 64 bits and accumulate them in the checksum
			checksum ^= Hash(word);
			// Reset word
			word = 0ULL;
		}
	}
	// Hash the last word to account for file sizes that are not a multiple of wordSize
	checksum ^= Hash(word);
	
	return checksum;
*/

void FunctionLoader::LoadNewDLL(uint64_t hash)
{
	if (m_FuncPtr)
	{
		// Undefines the function pointer
		// If anything fails after here, it will not point to a freed section of memory
		m_FuncPtr = nullptr;
		// Free the old DLL handle
		FreeLibrary(m_DllHandle);
		// Delete the old DLL file
		DeleteFileA(m_DllTargetName);
		// Set DLL hash to zero
		m_DllHash = 0ULL;
	}

	// Generate a file name using random Base64 characters based on the DLL hash
	const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	for (int i = 0; i < 10; i++)
		// Use every 6 bits of the hash value to get a random Base64 character
		m_DllTargetName[i + 4] = b64[(hash >> (i * 6)) & 0b111111];

	// If the 'gen' folder doesn't exist, create it
	// Ignore errors in order to proceed even if the folder already exists
	// If some other error occurs, 'CopyFileA' will return an error anyway
	CreateDirectoryA("gen", nullptr);

	// Copy the DLL into a new file
	if (!CopyFileA(m_DllName, m_DllTargetName, false))
		return;

	// Load the new DLL file
	m_DllHandle = LoadLibraryA(m_DllTargetName);
	if (!m_DllHandle)
	{
		DeleteFileA(m_DllTargetName);
		return;
	}

	// Get the function pointer
	m_FuncPtr = (void*)GetProcAddress(m_DllHandle, m_FuncName);
	if (!m_FuncPtr)
	{
		FreeLibrary(m_DllHandle);
		DeleteFileA(m_DllTargetName);
		return;
	}

	// Set DLL hash at the end, after everything else worked correctly,
	// in order to avoid a situation in which a new DLL hasn't been loaded properly,
	// but 'm_DllHash' has already been set to the new DLL's hash,
	// causing the program to think that it doesn't need to load the new DLL file
	m_DllHash = hash;
}

/*
bool FunctionLoader::LoadFunction()
{
	// Create target DLL name
	sprintf(m_DllTargetName, "%020llu.dll", m_Checksum);

	// Open target DLL file in write mode
	FILE* target = fopen(m_DllTargetName, "wb");
	if (!target)
	{
		printf("Failed to create target DLL file.\n");
		return false;
	}

	// Write buffer to target DLL file
	size_t written = fwrite(m_DllBuffer, 1, m_DllBufferSize, target);
	if (written != m_DllBufferSize)
	{
		printf("Failed to write to target DLL.\n");
		return false;
	}

	fclose(target);

	// Load DLL file
	m_DllHandle = LoadLibraryA(m_DllTargetName);
	if (!m_DllHandle)
	{
		printf("Failed to load DLL.\n");
		return false;
	}

	// Get the function address
	m_FuncPtr = (void*)GetProcAddress(m_DllHandle, m_FuncName);
	if (!m_FuncPtr)
	{
		printf("Failed to get function address.\n");
		FreeLibrary(m_DllHandle);
		return false;
	}

	return true;
}
*/

uint64_t FunctionLoader::Hash(uint64_t n)
{
	// Based on Google's implementation of Murmur3 hash
	uint64_t x = n;
	x *= 0x9ddfea08eb382d69ULL;
	x ^= (x >> 47);
	x ^= n;
	x *= 0x9ddfea08eb382d69ULL;
	x ^= (x >> 47);
	x *= 0x9ddfea08eb382d69ULL;
	return x;
}

