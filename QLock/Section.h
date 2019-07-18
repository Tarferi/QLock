#pragma once

#include "ReadBuffer.h"
#include "WriteBuffer.h"
#include "common.h"

class Section {
public:
	Section(unsigned char* name, unsigned int size, ReadBuffer* buffer);
	virtual ~Section();
	bool process();

	char* getName() {
		return this->name;
	}

	virtual bool write(WriteBuffer* buffer) = 0;
	unsigned int bufferBeginPosition;
	unsigned int size;

protected:
	ReadBuffer* buffer;
	char* name;
	
	virtual bool parse() = 0;


private:
	bool processed = false;
	
};

class BasicSection : public Section {

public:

	COMMON_CONSTR_SEC(BasicSection)

	virtual ~BasicSection() {
		if (this->data != nullptr) {
			free(this->data);
			this->data = nullptr;
		}
	}

protected:
	bool parse() {
		bool error = false;
		this->data = this->buffer->readArray(this->size, &error);
		return !error;
	}

	bool write(WriteBuffer* buffer) {
		bool error = false;
		buffer->writeArray(this->data, this->size, &error);
		return !error;
	}

private:
	unsigned char* data = nullptr;

};

class Section_STR_ : public Section {

public:

	char* getRawString(unsigned int index) {
		index--;
		if (index < this->strings.getSize()) {
			return this->strings.get(index);
		}
		else {
			return "";
		}
	}

	unsigned short getNewStringIndex(char* string) {
		GET_CLONED_STRING(newString, string, { return 0; });
		this->strings.append(newString);
		return this->strings.getSize();
	}

	void setRawString(unsigned int index, char* string) {
		index--;
		if (index < this->strings.getSize()) {
			char* oldString = this->strings.get(index);
			GET_CLONED_STRING(newString, string, { return; });
			this->strings.set(index, newString);
			free(oldString);
		}
	}

	COMMON_CONSTR_SEC(Section_STR_)

	virtual ~Section_STR_() {
		for (unsigned int stringIndex = 0; stringIndex < strings.getSize(); stringIndex++) {
			char* string = strings[stringIndex];
			free(string);
		}
	}

protected:
	bool parse() {

		bool error = false;
		unsigned int totalStrings = this->buffer->readShort(&error);
		if (error) {
			return false;
		}
		Array<unsigned short> offsets;
		for (unsigned int i = 0; i < totalStrings; i++) {
			unsigned short offset = this->buffer->readShort(&error);
			if (error) {
				return false;
			}
			offsets.append(offset);
		}
		unsigned int off = 2 + (2 * totalStrings);
		unsigned int dataSize = this->size - off;
		unsigned char* data = this->buffer->readArray(dataSize, &error);
		if (error) {
			if (data != nullptr) {
				free(data);
			}
			return false;
		}
		for (unsigned int i = 0; i < totalStrings; i++) {
			unsigned short offset = offsets[i];
			offset -= off;
			if (offset < this->size) {
				unsigned int len = 0;
				for (unsigned int i = offset; i < dataSize; i++, len++) {
					if (data[i] == 0) {
						break;
					}
				}
				MALLOC_N(newString, char, len + 1, { free(data); return false; });
				memcpy(newString, data + offset, len);
				newString[len] = 0;
				this->strings.append(newString);
			}
			else {
				MALLOC_N(emptyString, char, 1, { free(data); return false; });
				emptyString[0] = 0;
				this->strings.append(emptyString);
			}
		}
		free(data);
		return true;
	}

	bool write(WriteBuffer* buffer) {
		bool error = false;
		unsigned short totalStrings = this->strings.getSize();
		buffer->writeShort(totalStrings,&error);
		if (error) {
			return false;
		}
		unsigned int off = 2 + (2 * totalStrings);
		unsigned int begin = off;
		for (unsigned int i = 0; i<totalStrings; i++) {
			buffer->writeShort(begin, &error);
			if (error) {
				return false;
			}
			begin += strlen(this->strings[i]) + 1;
		};
		for (unsigned int i = 0; i < totalStrings; i++) {
			buffer->writeZeroDelimitedString((unsigned char*) this->strings.get(i), &error);
			if (error) {
				return false;
			}
		}
		return true;
	}

private:

	Array<char*> strings;

};

typedef struct Action {

	unsigned int SourceLocation;
	unsigned int TriggerText;
	unsigned int WAVStringNumber;
	unsigned int Time;
	unsigned int Player;
	unsigned int Group;
	unsigned short UnitType;
	unsigned char ActionType;
	unsigned char UnitsNumber;
	unsigned char Flags;
	unsigned char Unused[3];

} Action;

typedef struct Condition {
	unsigned int locationNumber;
	unsigned int groupNumber;
	unsigned int Quantifier;
	unsigned short UnitID;
	unsigned char Comparision;
	unsigned char ConditionType;
	unsigned char Resource;
	unsigned char Flags;
	unsigned short Unused;

} Condition;

typedef struct Trigger {
	
	Condition conditions[16];
	Action actions[64];

	unsigned int flags;
	unsigned char players[28];

} Trigger;

class Section_TRIG : public Section {

public:

	COMMON_CONSTR_SEC(Section_TRIG)

		virtual ~Section_TRIG();

	Array<Trigger*> triggers;

protected:

	bool parse() {

		unsigned int totalTriggers = this->size / sizeof(Trigger);
		bool error = false;
		for (unsigned int i = 0; i < totalTriggers; i++) {
			Trigger* trigger = (Trigger*) this->buffer->readArray(sizeof(Trigger), &error);
			if (error) {
				return false;
			}
			this->triggers.append(trigger);
		}
		return true;
	}

	bool write(WriteBuffer* buffer) {
		bool error = false;
		for (unsigned int i = 0; i < this->triggers.getSize(); i++) {
			Trigger* trigger = this->triggers[i];
			buffer->writeArray((unsigned char*)trigger, sizeof(Trigger), &error);
			if (error) {
				return false;
			}
		};
		return true;
	}



};

class Section_MBRF : public Section_TRIG {
public:
	Section_MBRF(unsigned char* name, unsigned int size, ReadBuffer* buffer) : Section_TRIG(name, size, buffer) { }

};

class Section_SPRP : public Section {

public:

	COMMON_CONSTR_SEC(Section_SPRP)

	bool parse() {
		bool error = false;
		str_scenarioName = this->buffer->readShort(&error);
		if (error) {
			return false;
		}
		str_scenarioDescription = this->buffer->readShort(&error);
		return !error;
	}

	bool write(WriteBuffer* buffer) {
		bool error = false;
		buffer->writeShort(this->str_scenarioName, &error);
		if (error) {
			return false;
		}
		buffer->writeShort(this->str_scenarioDescription, &error);
		return !error;
	}

	unsigned short str_scenarioName;
	unsigned short str_scenarioDescription;


};
