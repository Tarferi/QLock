// QLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string>
#include "Common.h"
#include "CHK.h"
#include "Storm.h"
#include <ctime>

#define GET_SECT(type, name, source, sourceName) type* name = (type*) source->getSection(sourceName);if(!name){return false;}

bool replaceTimeFragment(char* string, char* lookup, char* replacement) {
	char* strPos = strstr(string, lookup);
	if (strPos != nullptr) {
		unsigned int offset = strPos - string;
		unsigned int stringLength = strlen(string);
		unsigned int lookupLength = strlen(lookup);
		unsigned int replacementLength = strlen(replacement);
		unsigned int offsetLength = stringLength - offset;
		memcpy(strPos, replacement, replacementLength);
		memcpy(strPos + replacementLength, strPos + lookupLength, offsetLength - (lookupLength - replacementLength));
		return true;
	}
	return false;
}

bool replaceTime(char* string, char type, unsigned long timeX) {
	unsigned char M_str[5];
	unsigned char D_str[5];
	unsigned char Y_str[7];
	unsigned char H_str[5];
	unsigned char m_str[5];
	unsigned char S_str[5];

	unsigned short M = 0;
	unsigned short D = 0;
	unsigned short Y = 0;
	unsigned short H = 0;
	unsigned short m = 0;
	unsigned short S = 0;

	time_t tm = timeX;
	std::tm time;
	localtime_s(&time, &tm);
	M = time.tm_mon + 1;
	D = time.tm_mday;
	Y = 1900 + time.tm_year;
	H = time.tm_hour;
	m = time.tm_min;
	S = time.tm_sec;

	sprintf_s((char*)M_str, 5, "%02d", M);
	sprintf_s((char*)D_str, 5, "%02d", D);
	sprintf_s((char*)Y_str, 7, "%04d", Y);
	sprintf_s((char*)H_str, 5, "%02d", H);
	sprintf_s((char*)m_str, 5, "%02d", m);
	sprintf_s((char*)S_str, 5, "%02d", S);

	GET_CLONED_STRING(M_LOOK, "%MMX%", { return false; });
	GET_CLONED_STRING(D_LOOK, "%DDX%", { free(M_LOOK); return false; });
	GET_CLONED_STRING(Y_LOOK, "%YYYYX%", { free(D_LOOK); free(M_LOOK); return false; });
	GET_CLONED_STRING(H_LOOK, "%HHX%", { free(Y_LOOK); free(D_LOOK); free(M_LOOK); return false; });
	GET_CLONED_STRING(m_LOOK, "%mmX%", { free(m_LOOK); free(Y_LOOK); free(D_LOOK); free(M_LOOK); return false; });
	GET_CLONED_STRING(S_LOOK, "%SSX%", { free(S_LOOK); free(m_LOOK); free(Y_LOOK); free(D_LOOK); free(M_LOOK); return false; });

	M_LOOK[3] = type;
	D_LOOK[3] = type;
	Y_LOOK[5] = type;
	H_LOOK[3] = type;
	m_LOOK[3] = type;
	S_LOOK[3] = type;

	bool repl = false;
	bool error = false;
	repl |= replaceTimeFragment(string, M_LOOK, (char*)M_str);
	repl |= replaceTimeFragment(string, D_LOOK, (char*)D_str);
	repl |= replaceTimeFragment(string, Y_LOOK, (char*)Y_str);
	repl |= replaceTimeFragment(string, H_LOOK, (char*)H_str);
	repl |= replaceTimeFragment(string, m_LOOK, (char*)m_str);
	repl |= replaceTimeFragment(string, S_LOOK, (char*)S_str);


	free(M_LOOK);
	free(D_LOOK);
	free(Y_LOOK);
	free(H_LOOK);
	free(m_LOOK);
	free(S_LOOK);

	return repl;
}

char* transformUserString(char* string, bool* error) {
	GET_CLONED_STRING(src, "<01>", { *error = true; return 0; });
	GET_CLONED_STRING(dst, "\x01", { free(src); *error = true; return 0; });

	for (unsigned short i = 0; i <= 0x1f; i++) {
		dst[0] = (char)i;
		dst[1] = (char)0;
		sprintf_s(src, 5, "<%02x>", i);
		while (replaceTimeFragment(string, src, dst)) {};
		sprintf_s(src, 5, "<%02X>", i);
		while (replaceTimeFragment(string, src, dst)) {};
	}

	replaceTimeFragment(string, (char*) "<01>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<02>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<03>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<04>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<05>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<06>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<07>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<08>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<09>", (char*)"\x01");
	replaceTimeFragment(string, (char*) "<0A>", (char*)"\x01");

	free(src);
	free(dst);

	return string;
}

bool addTimeLockTrigger(Section_TRIG* v2TRIG, unsigned char comparator, unsigned short stringIndex, unsigned int time) {
	MALLOC_N(newTrig, Trigger, 1, { return false; });
	memset(newTrig, 0, sizeof(Trigger));
	newTrig->flags = 0;
	newTrig->players[17] = 1; // All players
	newTrig->conditions[0].ConditionType = 15; // Deaths
	newTrig->conditions[0].groupNumber = 334581; // Seconds
	newTrig->conditions[0].Comparision = comparator; // At most / at least
	newTrig->conditions[0].Quantifier = time; // Max allowed time

	newTrig->actions[0].ActionType = 9; // Display text message
	newTrig->actions[0].Flags = 4;
	newTrig->actions[0].TriggerText = stringIndex; // Display text message

	newTrig->actions[1].ActionType = 4; // Wait
	newTrig->actions[1].Time = 5000; // Display text message

	newTrig->actions[2].ActionType = 2; // Defeat

	if (v2TRIG->triggers.append(newTrig)) {
		return true;
	}
	else {
		return false;
	}
}

unsigned long getTimeDiff(char* str, bool* error) {
	unsigned long minute = 60;
	unsigned long hour = 60 * minute;
	unsigned long day = 24 * hour;
	unsigned long month = 30 * day; // Don't judge
	unsigned long year = 365 * day; // The extra day doesn't really matter

	int len = strlen(str);
	GET_CLONED_STRING_LEN(xstr, str, len + 15, { *error = true; return 0; }); // Extra space
	xstr[len + 0] = ':';
	xstr[len + 1] = '0';
	xstr[len + 2] = ':';
	xstr[len + 3] = '0';
	xstr[len + 4] = ':';
	xstr[len + 5] = '0';
	xstr[len + 6] = ':';
	xstr[len + 7] = '0';
	xstr[len + 8] = ':';
	xstr[len + 9] = '0';
	xstr[len + 10] = ':';
	xstr[len + 11] = '0';
	xstr[len + 12] = ':';
	xstr[len + 13] = 0;
	char* toFree = xstr;

#define TIMEHELP(i) ends[i] = strstr(xstr, ":");ends[i][0] = (char)0;ends[i] = xstr;xstr = xstr + (strlen(ends[i])) + 1; iends[i] = atoi(ends[i]);

	char* ends[6];
	int iends[6];

	TIMEHELP(0);
	TIMEHELP(1);
	TIMEHELP(2);
	TIMEHELP(3);
	TIMEHELP(4);
	TIMEHELP(5);

	free(toFree);

	unsigned long result = iends[5]; // Seconds
	result += iends[4] * minute; // Minutes
	result += iends[3] * hour; // Hours
	result += iends[2] * day; // Days
	result += iends[1] * month; // Months
	result += iends[0] * year; // Years
	LOG_INFO("STRING PROCESSOR", "Decoded string \"%s\", to %d years, %d months, %d days, %d hours, %d minutes and %d seconds", str, iends[0], iends[1], iends[2], iends[3], iends[4], iends[5])
		return result;
}

bool AddTimeLockTriggers(CHK* v2, char* timeLockFrom, char* timeLockTo, char* message, bool replaceVariablesInTriggers) {
	GET_SECT(Section_STR_, v2STR, v2, "STR ");
	GET_SECT(Section_TRIG, v2TRIG, v2, "TRIG");
	GET_SECT(Section_SPRP, v2SPRP, v2, "SPRP");

	unsigned long now = (unsigned long)std::time(nullptr);

	bool error = false;

	unsigned int timeFrom = getTimeDiff(timeLockFrom, &error);
	if (error) {
		return false;
	}
	unsigned int timeTo = getTimeDiff(timeLockTo, &error);
	if (error) {
		return false;
	}

	unsigned long lockFrom = now - timeFrom;
	unsigned long lockTo = now + timeTo;

	GET_CLONED_STRING(userString, message, { return false; });
	replaceTime(userString, 'F', lockFrom);
	replaceTime(userString, 'T', lockTo);
	LOG_INFO("TRIGGER PROCESSOR", "Now is %d, lock begin is %d and lock and is %d, lock message is %s", now, lockFrom, lockTo, userString);
	if (transformUserString(userString, &error) == nullptr) {
		free(userString);
		return false;
	}
	if (error) {
		free(userString);
		return false;
	}



	unsigned char AT_LEAST = 0;
	unsigned char AT_MOST = 1;

	bool replacedFrom = false;
	bool replacedTo = false;
	for (unsigned int i = 0; i < v2TRIG->triggers.getSize(); i++) {
		Trigger* trigger = v2TRIG->triggers.get(i);
		for (unsigned int o = 0; o < 16; o++) {
			Condition* condition = &(trigger->conditions[o]);
			if (condition->ConditionType == 15 && condition->groupNumber == 334581) { // Previous timelock trigger
				Action* action = &(trigger->actions[0]);
				if (condition->Comparision == AT_LEAST) { // At least
					condition->Quantifier = lockTo;
					replacedTo = true;
					v2STR->setRawString(action->TriggerText, userString);
					LOG_INFO("TRIGGER PROCESSOR", "Found previous begin interval trigger, replacing message");
					if (replacedFrom) {
						break;
					}
				}
				else if (condition->Comparision == AT_MOST) { // At most
					condition->Quantifier = lockFrom;
					replacedFrom = true;
					LOG_INFO("TRIGGER PROCESSOR", "Found previous end interval trigger, replacing message");
					v2STR->setRawString(action->TriggerText, userString);
					if (replacedTo){
						break;
					}
				}
			}
		}
	}
	unsigned short userStringIndex = 0;
	if (!replacedFrom) {
		userStringIndex = v2STR->getNewStringIndex(userString);
		addTimeLockTrigger(v2TRIG, AT_LEAST, userStringIndex, lockTo);
	}
	if (!replacedTo) {
		if (userStringIndex == 0) {
			userStringIndex = v2STR->getNewStringIndex(userString);
		}
		addTimeLockTrigger(v2TRIG, AT_MOST, userStringIndex, lockFrom);
	}
	free(userString);

	if(replaceVariablesInTriggers){
		for (unsigned int i = 0; i < v2TRIG->triggers.getSize(); i++) {
			Trigger* trigger = v2TRIG->triggers.get(i);
			for (unsigned int o = 0; o < 64; o++) {
				Action* action = &(trigger->actions[o]);
				if (action->TriggerText != 0) {
					char* string = v2STR->getRawString(action->TriggerText);
					if (replaceTime(string, 'F', lockFrom) || replaceTime(string, 'T', lockTo)) {
						v2STR->setRawString(action->TriggerText, string);
						LOG_INFO("TRIGGER PROCESSOR", "Replacing string index %d found in action %d of trigger %d to \"%s\"", action->TriggerText, o, i, string);
					}
				}
			}
		}
	}
	char* scenarioDescription = v2STR->getRawString(v2SPRP->str_scenarioDescription);
	if (replaceTime(scenarioDescription, 'F', lockFrom) || replaceTime(scenarioDescription, 'T', lockTo)) {
		GET_CLONED_STRING(scenarioDescription_cloned, scenarioDescription, { return false; });
		v2STR->setRawString(v2SPRP->str_scenarioDescription, scenarioDescription_cloned);
		LOG_INFO("TRIGGER PROCESSOR", "Replacing scenarion description to \"%s\"", scenarioDescription);
		free(scenarioDescription_cloned);
	}
	return true;
}

int main(int argc, char** argv) {

	bool skip = false;
	char* inputFile = nullptr;
	char* outputFile = nullptr;
	char* userString = nullptr;
	char* fromString = nullptr;
	char* toString = nullptr;
	bool showHelp = true;
	bool replaceInTriggers = false;
	
#define ARG_IS(longVersion, shortVersion) !strcmp(arg, shortVersion) || !strcmp(arg, longVersion)
#define SET_ARG(item, err) if(item != nullptr || argI + 1 == argc){fprintf(stderr, err "\n");return 2;};item = argv[argI + 1];skip=true;showHelp = false;continue;
#define ARG(longVersion, shortVersion, item, err) if(ARG_IS(longVersion, shortVersion)) {SET_ARG(item,err);}

	for (int argI = 1; argI < argc; argI++) {
		if (skip) {
			skip = false;
			continue;
		}
		char* arg = argv[argI];
		ARG("-i", "--input", inputFile, "Invalid option for input file");
		ARG("-o", "--output", outputFile, "Invalid option for output file");
		ARG("-f", "--from", fromString, "Invalid option for interval definition");
		ARG("-t", "--to", toString, "Invalid option for interval definition");
		ARG("-m", "--message", userString, "Invalid option for message");
		if (ARG_IS("-h", "--help")) {
			showHelp = true;
			break;
		}
		if (ARG_IS("-r", "--replace")) {
			replaceInTriggers = true;
			continue;
		}
	}
	if (showHelp) {
		fprintf(stderr, "Starcraft Map Time locker by iThief\r\n");
		fprintf(stderr, "\r\n\r\nUsage:\r\n");
		fprintf(stderr, "\t-i <input_file>      Input map file\r\n");
		fprintf(stderr, "\t-o <output_file>     Output map file (can be the same as output)\r\n");
		fprintf(stderr, "\t-f <unlock_begin>    Relative specification of unlock begin (see below)\r\n");
		fprintf(stderr, "\t-t <unlock_begin>    Relative specification of unlock begin (see below)\r\n");
		fprintf(stderr, "\t-m <message>         Message to display when map is locked (see below)\r\n");
		fprintf(stderr, "\t-r                   Replace time variables in trigger actions \r\n");
		fprintf(stderr, "\r\n\r\nDate format:\r\n");
		fprintf(stderr, "\t<Years>:<Months>:<Days>:<Hours>:<Minutes>:<Seconds>\r\n");
		fprintf(stderr, "\r\n\r\nDate Example:\r\n");
		fprintf(stderr, "\t\"0:0:-1:0:0:0     Means yesterday at this time (months = 30 days)\"\r\n");
		fprintf(stderr, "\r\n\r\nMessage format:\r\n");
		fprintf(stderr, "\t\"This uses Scmdraft string format (See Scmdraft string editor)\"\r\n");
		fprintf(stderr, "\t\"Message can include variables %%YYYY[F|T]%%, %%MM[F|T]%%, %%DD[F|T]%%, %%HH[F|T]%%, %%mm[F|T]%%, %%SS[F|T]%%   where F means From and T means To\"\r\n");
		fprintf(stderr, "\r\n\r\nExample:\r\n");
		fprintf(stderr, "\tQlock.exe -i my_cool_map.scx -o my_cool_map_locked.scx -f 0:0:-1:0:0:0 -t 0:0:1:0:0:0 -m \"Yo I made this map time-locked. It will be unplayable on %%MMT%%.%%DDT%% at %%mmT%%:%%ssT%%.\"\r\n");
		fprintf(stderr, "\r\nAbove example command will product my_cool_map_locked.scx that will be only playable for a day (timezones might vary). When it's no longer playable, it will display a message telling people until when it was playable, give them 5 seconds to read it and then defeat for all players.\r\n");
		fprintf(stderr, "\r\nIt's not recommended to use it in loop without repairing STR section (editing in editor or something).\r\n");
		return 3;
	}

	bool err = false;
	Storm storm(&err);
	if (err) {
		LOG_ERROR("LOCK", "Cannot decode storm library");
		return 1;
	}
	MapFile* mf = storm.readSCX(inputFile, &err);
	if (err) {
		LOG_ERROR("LOCK", "Cannot open \"%s\"", inputFile);
		return 1;
	}
	if (!AddTimeLockTriggers(mf->getCHK(), fromString, toString, userString, replaceInTriggers)) {
		LOG_ERROR("LOCK", "Could not process file \"%s\"", inputFile);
		delete mf;
		return 1;
	}
	if (!storm.writeSCX(outputFile, mf)) {
		LOG_ERROR("LOCK", "Cannot write \"%s\"", outputFile);
		return 1;
	}
	delete mf;
	LOG_INFO("LOCK", "Done");
    return 0;
}

