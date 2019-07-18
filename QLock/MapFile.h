#pragma once
#include "common.h"
#include "CHK.h"

class Storm;

class MapFile {
public:
	MapFile(char* originalInputFile, Array<char*>* data, Array<unsigned int>* dataLengths, Array<char*>* fileNames, bool* error);
	~MapFile();

	CHK* getCHK();
	void writeToFile(Storm* storm, char* name, bool* error);

	Array<char*>* contents;
	Array<unsigned int>* dataLengths;
	Array<char*>* fileNames;

	char* originalInputFile;

private:
	CHK* chk;


};

