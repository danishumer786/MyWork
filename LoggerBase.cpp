#include <filesystem>

#include "LoggerBase.h"
#include "../CommonFunctions.h"
#include "../Security/DataDecryptor.h"
#include "../ErrorMessageStream.h"
#include "../WindowsFunctions.h"

using namespace std;


string GetEncryptedLinePrefix() {
	return "---";
}

string GetMetadataLinePrefix() {
	return ">>> ";
}



LoggerBase::LoggerBase() {
	AddColumn("Date");
	AddColumn("Time");
}


//-------------------------------------------------------------------------
// Default log output file

void LoggerBase::SetFilePath(const string& file_path) {
	setFilePathSuccessful = false;

	if (PathIsValid(file_path))
		filePath = file_path;
	else {
		e << "Invalid file path: \"" << file_path << "\"." << endl;
		return;
	}

	logFile.open(filePath, ios::app);
	if (logFile.is_open()) {
		setFilePathSuccessful = true;
		logFile.close();
	}
	else {
		e << "Failed to open log file: \"" << file_path << "\"." << endl;
	}
}

string LoggerBase::GetLogFilePath() const {
	return filePath;
}

bool LoggerBase::SetFilePathSuccessful() const {
	return setFilePathSuccessful;
}


//-------------------------------------------------------------------------
// Saving log contents to new file at any time

void LoggerBase::SaveMemoryLogToFile(const string& file_path) {
	SaveMemoryLogToFileHelper(file_path, false);
}

void LoggerBase::SaveMemoryLogToFileEncrypted(const string& file_path) {
	SaveMemoryLogToFileHelper(file_path, true);
}

bool LoggerBase::SaveSuccessful() const {
	return saveToFileSuccessful;
}


//-------------------------------------------------------------------------
// Logging structured data

void LoggerBase::AddColumn(const string& columnName) {
	columnNames.push_back(columnName);
}

void LoggerBase::WriteHeaderLine() {
	string header = "";
	for (string columnName : columnNames)
		header += columnName + ",";

	// Remove trailing comma
	if (header.length() > 1)
		header[header.length() - 1] = ' ';

	if (encryptData)
		CommitLineEncrypt(header);
	else
		CommitLine(header);
}

void LoggerBase::LogDataPoint(const vector<string>& values) {

	if (values.size() != columnNames.size() - 2) {
		e << "ERROR - Logging function: Number of values to write does not match number of columns." << endl;
		return;
	}

	string date = GenerateDateString();
	string time = GenerateTimeString();

	// Notify all subscribed observers with data to be logged
	map<string, string> dataForObservers;
	dataForObservers["Date"] = date;
	dataForObservers["Time"] = time;
	for (size_t col = 2; col < columnNames.size(); col++) {
		dataForObservers[columnNames[col]] = values[col - 2];
	}
	dataPointLogged(dataForObservers);


	string line = "";

	line += date + ",";
	line += time + ",";

	for (string value : values)
		line += value + ",";

	if (line.length() > 0)
		line[line.length() - 1] = ' ';

	if (encryptData)
		CommitLineEncrypt(line);
	else
		CommitLine(line);



	/*if (values.size() != columnNames.size() - 2) {
		e << "ERROR - Logging function: Number of values to write does not match number of columns." << endl;
		return;
	}

	string line = "";

	line += GenerateDateString() + ",";
	line += GenerateTimeString() + ",";

	for (string value : values)
		line += value + ",";

	if (line.length() > 0)
		line[line.length() - 1] = ' ';

	if (encryptData)
		CommitLineEncrypt(line);
	else
		CommitLine(line);*/
}


//-------------------------------------------------------------------------
// Logging custom data

void LoggerBase::CommitLine(string line) {
	logDataInMemory.push_back(line);
	WriteLineToFile(line);
}

void LoggerBase::CommitLineEncrypt(string line) {
	logDataInMemory.push_back(line);
	WriteEncryptedLineToFile(line);
}

void LoggerBase::CommitLineMetadata(string line) {
	line = GetMetadataLinePrefix() + line;
	logDataInMemory.push_back(line);
	WriteLineToFile(line);
}


//-------------------------------------------------------------------------
// Other

string LoggerBase::AppendNewLineIfNecessary(string line) {
	if (line == "")
		return "";
	if (line.back() != '\n')
		line = line + '\n';
	return line;
}

void LoggerBase::WriteLineToFile(string line) {
	logFile.open(filePath, ios::app);
	if (logFile.is_open()) {
		line = AppendNewLineIfNecessary(line);
		logFile << line;
		logFile.close();
	}
	else {
		e << "Failed to commit line \"" << line << "\" to file \"" << filePath << "\"" << endl;
	}
}

void LoggerBase::WriteEncryptedLineToFile(string line) {
	line = GetEncryptedLinePrefix() + cryptofy(line);
	WriteLineToFile(line);
}

void LoggerBase::SaveMemoryLogToFileHelper(const string& file_path, bool encrypt) {
	saveToFileSuccessful = false;
	if (!PathIsValid(file_path)) {
		e << "Invalid file path: \"" << file_path << "\"." << endl;
		return;
	}

	ofstream file(file_path);
	if (file.is_open()) {
		for (string line : logDataInMemory) {
			if (encrypt)
				line = GetEncryptedLinePrefix() + cryptofy(line);
			line = AppendNewLineIfNecessary(line);
			file << line;
		}
		file.close();
		saveToFileSuccessful = true;
	}
	else {
		e << "Failed to save to file \"" << file_path << "\".";
	}
}

void LoggerBase::SetEncryptOption(const bool encrypt_data) {
	encryptData = encrypt_data;
}

void LoggerBase::Reset() {
	filePath = "";
	logDataInMemory.clear();
	columnNames.clear();
	setFilePathSuccessful = false;
	saveToFileSuccessful = false;
}
