#pragma once

#include "common.h"
#include "Section.h"
#include "ReadBuffer.h"

class MapFile;

class CHK
{
public:
	CHK(char* data, unsigned int size);
	~CHK();
	bool write(WriteBuffer* buffer);
	Section* getSection(const char* name);
	Array<Section*> sections;
	bool isValid() {
		return this->valid;
	}
private:
	
	ReadBuffer* buffer;
	bool valid;
	bool parse();
};

