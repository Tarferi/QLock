#include "CHK.h"
#include "common.h"

CHK::CHK(char * data, unsigned int size) {
	bool error = false;
	this->buffer = new ReadBuffer((unsigned char*) data, size, &error);
	if (error) {
		this->valid = false;
		return;
	}
	this->valid = this->parse();
}

CHK::~CHK() {
	for (unsigned int i = 0; i < this->sections.getSize(); i++) {
		Section* section = this->sections[i];
		if (section != nullptr) {
			if (!strcmp(section->getName(), "TRIG")) {
				delete section;
				continue;
			}
			delete section;
		}
		section = nullptr;
	};
	delete this->buffer;
}

bool CHK::write(WriteBuffer* buffer) {
	for (unsigned int i = 0; i < this->sections.getSize(); i++) {
		Section* section = this->sections[i];
		bool error = false;
		buffer->writeFixedLengthString((unsigned char*) section->getName(), &error);
		if (error) {
			LOG_ERROR("CHK", "Failed to write section \"%s\"", section->getName());
			return false;
		}
		unsigned int prePosition = buffer->getPosition();
		buffer->writeInt(0, &error); // Later replace this with size
		if (error) {
			LOG_ERROR("CHK", "Failed to write section \"%s\"", section->getName());
			return false;
		}
		section->write(buffer);
		unsigned int postPosition = buffer->getPosition();
		unsigned int sectionSize = (postPosition - prePosition) - 4;
		buffer->setPosition(prePosition);
		buffer->writeInt(sectionSize, &error);
		if (error) {
			LOG_ERROR("CHK", "Failed to write section \"%s\"", section->getName());
			return false;
		}
		buffer->setPosition(postPosition);
	}
	return true;
}

Section * CHK::getSection(const char * name) {
	for (unsigned int i = 0; i < this->sections.getSize(); i++) {
		Section* section = this->sections[i];
		char* sname = section->getName();
		if (!strcmp(name, sname)) {
			return section;
		}
	};
	return nullptr;
}

bool CHK::parse() {
	LOG_INFO("CHK", "BEGIN PARSING of %d bytes", this->buffer->getDataSize())
	while (!this->buffer->isDone()) {
		bool error = false;
		char* name =(char*) this->buffer->readFixedLengthString(4, &error);
		if (error) {
			LOG_ERROR("CHK", "Error reading section name");
			return false;
		}
		unsigned int size = this->buffer->readInt(&error);
		if (error) {
			LOG_ERROR("CHK", "Error reading section size");
			return false;
		}
		LOG_INFO("CHK", "Found section \"%s\" of size %d", name, size);
		Section* section;
		if (!strcmp(name, "STR ")) {
			section = new Section_STR_((unsigned char*)name, size, this->buffer);
		}
		else if (!strcmp(name, "SPRP")) {
			section = new Section_SPRP((unsigned char*)name, size, this->buffer);
		}
		else if (!strcmp(name, "TRIG")) {
			section = new Section_TRIG((unsigned char*)name, size, this->buffer);
		}
		else if (!strcmp(name, "MBRF")) {
			section = new Section_MBRF((unsigned char*)name, size, this->buffer);
		} else {
			section = new BasicSection((unsigned char*)name, size, this->buffer);
		}
		this->sections.append(section);
		if (!section->process()) {
			LOG_ERROR("CHK", "Failed to process section \"%s\" (size %d)", name, size);
			return false;
		}
	}
	return true;
}

