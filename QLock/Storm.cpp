#include "Storm.h"

bool decompressLib(char** data, unsigned int* size) {
	bool error = false;
	char* newData;
	unsigned int newSize;
	decompress((char*)_acStormLib, _acStormLibLen, &newData, &newSize, &error);
	*data = newData;
	*size = newSize;
	return !error;
}

Storm::Storm(bool* error) {
	this->lib = nullptr;
	unsigned int libSize;
	if (!decompressLib(&this->decompressedLib, &libSize)) {
		LOG_ERROR("STORM", "Decompression failed");
		*error = true;
		return;
	}

	this->lib = MemoryLoadLibrary(this->decompressedLib, libSize);
	if (!this->lib) {
		*error = true;
		return;
	}
	SFileOpenArchive = (SFileOpenArchiveF)MemoryGetProcAddress(this->lib, "SFileOpenArchive");
	SFileFindNextFile = (SFileFindNextFileF)MemoryGetProcAddress(this->lib, "SFileFindNextFile");
	SFileFindFirstFile = (SFileFindFirstFileF)MemoryGetProcAddress(this->lib, "SFileFindFirstFile");
	SFileFindClose = (SFileFindCloseF)MemoryGetProcAddress(this->lib, "SFileFindClose");
	SFileExtractFile = (SFileExtractFileF)MemoryGetProcAddress(this->lib, "SFileExtractFile");
	SFileReadFile = (SFileReadFileF)MemoryGetProcAddress(this->lib, "SFileReadFile");
	SFileOpenFileEx = (SFileOpenFileExF)MemoryGetProcAddress(this->lib, "SFileOpenFileEx");
	SFileCloseFile = (SFileCloseFileF)MemoryGetProcAddress(this->lib, "SFileCloseFile");
	SFileCreateFile = (SFileCreateFileF)MemoryGetProcAddress(this->lib, "SFileCreateFile");
	SFileWriteFile = (SFileWriteFileF)MemoryGetProcAddress(this->lib, "SFileWriteFile");
	SFileFinishFile = (SFileFinishFileF)MemoryGetProcAddress(this->lib, "SFileFinishFile");
	SFileCreateArchive = (SFileCreateArchiveF)MemoryGetProcAddress(this->lib, "SFileCreateArchive");
	SFileCloseArchive = (SFileCloseArchiveF)MemoryGetProcAddress(this->lib, "SFileCloseArchive");
	SFileRemoveFile = (SFileRemoveFileF)MemoryGetProcAddress(this->lib, "SFileRemoveFile");
	SFileAddFileEx = (SFileAddFileExF)MemoryGetProcAddress(this->lib, "SFileAddFileEx");
	SFileCompactArchive = (SFileCompactArchiveF)MemoryGetProcAddress(this->lib, "SFileCompactArchive");

	LOG_INFO("STORM", "Storm library initiated");
}

Storm::~Storm() {
	if (this->decompressedLib != nullptr) {
		free(this->decompressedLib);
		this->decompressedLib = nullptr;
	}
	if (this->lib != nullptr) {
		MemoryFreeLibrary(this->lib);
		this->lib = nullptr;
	}
	LOG_INFO("STORM", "Storm library uninitiated");
}

MapFile * Storm::readSCX(char * filePath, bool* error) {

	HANDLE mapFile;
	void* scx = SFileOpenArchive(filePath, 0, 0x00000100, &mapFile);
	if (!scx) {
		LOG_ERROR("STORM", "Failed to open file %s", filePath)
		*error = true;
		return nullptr;
	}
	LOG_INFO("STORM", "Opened file %s", filePath);

	SFILE_FIND_DATA data;

	HANDLE searchHandle = SFileFindFirstFile(mapFile, "*", &data, (const char*)0);

	Array<char*>* fileNames = new Array<char*>();
	Array<unsigned int>* fileSizes = new Array<unsigned int>();
	Array<char*>* filesContents = new Array<char*>();

	bool hasScenarioChk = false;

	do {
		
		fileSizes->append(data.dwFileSize);
		GET_CLONED_STRING(fileName, data.cFileName, { fileNames->freeItems(); filesContents->freeItems(); SFileFindClose(searchHandle); SFileCloseArchive(mapFile); delete fileNames; delete fileSizes; delete filesContents; return nullptr; });
		fileNames->append(fileName);
		HANDLE fileH;
		MALLOC_N(fileContents, char, data.dwFileSize, { fileNames->freeItems(); filesContents->freeItems(); SFileFindClose(searchHandle); SFileCloseArchive(mapFile); delete fileNames; delete fileSizes; delete filesContents; return nullptr; });
		SFileOpenFileEx(mapFile, data.cFileName, 0, &fileH);
		DWORD read;
		SFileReadFile(fileH, fileContents, data.dwFileSize, &read, 0);
		LOG_INFO("STORM", "Read file %s from %s", data.cFileName, filePath);
		if (read != data.dwFileSize) {
			LOG_ERROR("STORM", "Read only %d of %d total\n", read, data.dwFileSize);
		}
		SFileCloseFile(fileH);
		filesContents->append(fileContents);

	} while (SFileFindNextFile(searchHandle, &data));

	SFileFindClose(searchHandle);
	SFileCloseArchive(mapFile);
	LOG_INFO("STORM", "Closed file %s", filePath);
	MapFile* mf = new MapFile(filePath, filesContents, fileSizes, fileNames, error);
	return mf;
}

bool Storm::writeSCX(char* ffile, MapFile* mf) {
	GET_CLONED_STRING(file, ffile, {});
	for (unsigned int cI = 0; cI < strlen(file); cI++) {
		if (file[cI] == '\\') {
			file[cI] = '/';
		}
	}
	remove(file);
	{
		FILE* f;
		if (fopen_s(&f, mf->originalInputFile, "rb")) {
			LOG_ERROR("STORM", "Failed to open original file");
			return false;
		}
		bool error = false;
		ReadBuffer rb(f, &error);
		fclose(f);
		if (error || !rb.good) {
			return false;
		}
		unsigned int dataSize = rb.getDataSize();
		unsigned char* data = rb.readArray(dataSize, &error);
		WriteBuffer wb;
		wb.writeArray(data, dataSize, &error);
		if (error) {
			return false;
		}
		delete data;
		wb.writeToFile(file, &error);
		if (error) {
			return false;
		}
	}

	char* contents;
	unsigned int fileSize;
	WriteBuffer wb;
	CHK* chk = mf->getCHK();
	if (!chk->write(&wb)) {
		return false;
	}
	wb.getWrittenData((unsigned char**) (&contents), &fileSize);

	HANDLE mapFile;
	if(!SFileOpenArchive(file, 0, 0x00000200, &mapFile)) {
		LOG_ERROR("STORM", "Failed to open file %s", file)
		return false;
	}
	LOG_INFO("STORM", "Opened file %s for writing", file);


	HANDLE newFileH;
	if (!SFileCreateFile(mapFile, "staredit\\scenario.chk", 0, fileSize, 0, 0x00000200 | 0x80000000 | 0x12, &newFileH)) {
		SFileCloseArchive(mapFile);
		return false;
	}
	if (!SFileWriteFile(newFileH, contents, fileSize, 0)) {
		SFileFinishFile(newFileH);
		SFileCloseArchive(mapFile);
		return false;
	}
	if (!SFileFinishFile(newFileH)) {
		SFileCloseArchive(mapFile);
		return false;
	}
	//SFileCompactArchive(mapFile, 0, 0);
	SFileCloseArchive(mapFile);
	return true;
}
