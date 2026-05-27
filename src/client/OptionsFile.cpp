#include "OptionsFile.h"
#include <stdio.h>
#include <string.h>
#include <set>
#include <platform/log.h>

OptionsFile::OptionsFile() {
#ifdef __APPLE__
	settingsPath = "./Documents/options.txt";
#elif defined(ANDROID)
	settingsPath = "options.txt";
#else
	settingsPath = "options.txt";
#endif
}

void OptionsFile::save(const StringVector& settings) {
	std::set<std::string> keysToOverwrite;
	for (StringVector::const_iterator it = settings.begin(); it != settings.end(); ++it) {
		size_t colon = it->find(':');
		if (colon != std::string::npos)
			keysToOverwrite.insert(it->substr(0, colon));
	}

	StringVector preservedLines;
	FILE* input = fopen(settingsPath.c_str(), "r");
	if (input != NULL) {
		char lineBuff[256];
		while (fgets(lineBuff, sizeof lineBuff, input)) {
			size_t len = strlen(lineBuff);
			while (len > 0 && (lineBuff[len - 1] == '\n' || lineBuff[len - 1] == '\r'))
				lineBuff[--len] = '\0';
			if (len == 0)
				continue;

			char* colon = strchr(lineBuff, ':');
			if (colon != NULL) {
				std::string key(lineBuff, colon - lineBuff);
				if (keysToOverwrite.find(key) != keysToOverwrite.end())
					continue;
			}

			preservedLines.push_back(lineBuff);
		}
		fclose(input);
	}

	FILE* pFile = fopen(settingsPath.c_str(), "w");
	if(pFile != NULL) {
		for(StringVector::const_iterator it = settings.begin(); it != settings.end(); ++it) {
			fprintf(pFile, "%s\n", it->c_str());
		}
		for (StringVector::const_iterator it = preservedLines.begin(); it != preservedLines.end(); ++it) {
			fprintf(pFile, "%s\n", it->c_str());
		}
		fclose(pFile);
	} else {
		LOGI("OptionsFile::save failed to open '%s' for writing: %s", settingsPath.c_str(), strerror(errno));
	}
}

StringVector OptionsFile::getOptionStrings() {
	StringVector returnVector;
	FILE* pFile = fopen(settingsPath.c_str(), "r");
	if(pFile != NULL) {
		char lineBuff[128];
		while(fgets(lineBuff, sizeof lineBuff, pFile)) {
			// Strip trailing newline
			size_t len = strlen(lineBuff);
			while(len > 0 && (lineBuff[len-1] == '\n' || lineBuff[len-1] == '\r'))
				lineBuff[--len] = '\0';
			if(len < 3) continue;
			// Split "key:value" into two separate entries to match update() pairing
			char* colon = strchr(lineBuff, ':');
			if(colon) {
				returnVector.push_back(std::string(lineBuff, colon - lineBuff));
				returnVector.push_back(std::string(colon + 1));
			}
		}
		fclose(pFile);
	} else {
		LOGI("OptionsFile::getOptionStrings failed to open '%s' for reading: %s", settingsPath.c_str(), strerror(errno));
	}
	return returnVector;
}
