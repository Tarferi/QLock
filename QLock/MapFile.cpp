#include "MapFile.h"
#include "Storm.h"


MapFile::MapFile(char* originalFilename, Array<char*>* data, Array<unsigned int>* dataLengths, Array<char*>* fileNames, bool* error) {
	this->originalInputFile = originalFilename;
	this->chk = nullptr;
	this->contents = data;
	this->fileNames = fileNames;
	this->dataLengths = dataLengths;
}

MapFile::~MapFile() {
	if (this->chk != nullptr) {
		delete this->chk;
		this->chk = nullptr;
	}
	if (this->contents != nullptr) {
		for (unsigned int i = 0; i < this->contents->getSize(); i++) {
			char* content = this->contents->get(i);
			if (content != nullptr) {
				free(content);
				this->contents->set(i, nullptr);
			}
		}
		delete this->contents;
	}
	if (this->fileNames != nullptr) {
		for (unsigned int i = 0; i < this->fileNames->getSize(); i++) {
			if (this->fileNames->get(i) != nullptr) {
				free(this->fileNames->get(i));
			}
		}
		delete this->fileNames;
		this->fileNames = nullptr;
	}
	if (this->dataLengths != nullptr) {
		delete this->dataLengths;
	}
}

CHK * MapFile::getCHK()
{
	if (this->chk == nullptr) {
		for (unsigned int i = 0; i < this->fileNames->getSize(); i++) {
			if (!strcmp(this->fileNames->get(i), "staredit\\scenario.chk")) {
				this->chk = new CHK(this->contents->get(i), this->dataLengths->get(i));
			}
		}
	}
	return this->chk;
}

bool hasFile(MapFile* mf, char* name) {
	for (unsigned int wavIndex = 0; wavIndex < mf->fileNames->getSize(); wavIndex++) {
		char* wavFile = mf->fileNames->get(wavIndex);
		if (!strcmp(wavFile, name)) {
			return true;
		}
	}
	return false;
}

void MapFile::writeToFile(Storm* storm, char *name, bool* error) {
	
	// Create new mapfile and write it
	if (!storm->writeSCX(name, this)) {
		*error = true;
	}

}