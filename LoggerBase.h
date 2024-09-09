/**
* Logger Base : Writes structured data to a log file
*
* - Call AddColumn("COLUMN_NAME") to designate a comma-separated data attribute for each row.
* - The first two rows (date and time) are already automatically added for you.
* - After creating the logger and adding columns, pass in a vector of strings to LogDataPoint()
*		to commit a line of values to the log.
* - Can create many individual logger objects, each with its own columns.
*
* - Two methods of outputting data to log files:
*		- Logger creates a target log file upon construction writes to that file whenever
*			LogDataPoint(..) or CommitLine(..) is called.
*		- Logger also writes data to a vector of strings in memory. If you want to save this
*			data to another file besides the initial target, call SaveMemoryLogToFile(). 
*			This saves the data stored in memory to a new log file in addition to the
*			initial target file.
*
*
* Example usage:
*
*	string logfilepath = "C:/Users/jbutcher/Documents/myLog1.log";
*	LoggerBase logger;
*	logger.SetFilePath(logfilepath);
*
*	logger.AddColumn("PEC");
*	logger.AddColumn("PRF");
*
*	logger.WriteHeaderLine();
*
*	for (auto datapoint : data) {
*		vector<String> values = { datapoint.pec, datapoint.prf };
*		logger.LogDataPoint(values);
*	}
*
*	logger.SaveMemoryLogToFile();
*
* 
* @file LoggerBase.h
* @author James Butcher
* @created February 2023
* @version 1.0
*/
#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "LogNotifier.h"


#define LOG_API __declspec(dllexport)


// Logger objects prepend a special prefix to the front of all log lines
// written using CommitLineEncrypt(..). Use this function to get this
// prefix so log file readers can tell when a line needs to be decrypted.
LOG_API std::string GetEncryptedLinePrefix();
// Logger objects prepend a special prefix to the front of all log lines
// written using CommitLineMetaData(..). Use this function to get this 
// prefix so log file readers can tell when a line doesn't follow the
// data column format.
LOG_API std::string GetMetadataLinePrefix();


class LoggerBase : public LogNotifier {

public:
	// Create a logger to log to the target file path.
	//LOG_API LoggerBase(const std::string& file_path);
	LOG_API LoggerBase();


	//-------------------------------------------------------------------------
	// Default log output file

	// Choose a file path to receive logger output.
	// Must provide an absolute path string.
	LOG_API void SetFilePath(const std::string& file_path);
	LOG_API std::string GetLogFilePath() const;

	// Returns true if logger was able to create and open the log file at
	//   the target file path.
	LOG_API bool SetFilePathSuccessful() const;


	//-------------------------------------------------------------------------
	// Saving log contents to new file at any time

	// Save all data logged so far (including custom lines) to another file.
	LOG_API void SaveMemoryLogToFile(const std::string& file_path);
	LOG_API void SaveMemoryLogToFileEncrypted(const std::string& file_path);

	// Returns true if SaveMemoryLogToFile succeeded
	LOG_API bool SaveSuccessful() const;


	//-------------------------------------------------------------------------
	// Logging structured data

	// Add a data column for logging data points.
	//   - The column name provided will be printed in the header line.
	//   - Adding columns should be done all at once before logging data points,
	//     since the number of columns dictates the required number of values
	//     passed to the LogDataPoint(..) method.
	//   - Date and time columns are added automatically. You do not need to log them.
	LOG_API void AddColumn(const std::string& columnName);

	// Write the column headers separated by commas in a single line.
	//   - Should be done after adding columns but before logging data points.
	LOG_API void WriteHeaderLine();

	// Log a comma-separated row of values that correspond to the columns
	//   added previously.
	//   - The number of values must match the number of columns
	//   - The values must be in the same order as how the columns were added.
	//   - A date and a time value are added automatically to the beginning
	LOG_API void LogDataPoint(const std::vector<std::string>& values);


	//-------------------------------------------------------------------------
	// Logging custom data

	// Write a custom, unencrypted line to the file at any time.
	LOG_API void CommitLine(std::string line);
	// Write a custom line to the file at any time. 
	// It will get encrypted and then have a special prefix added.
	LOG_API void CommitLineEncrypt(std::string line);
	// Write a custom metadata line to the file at any time. 
	// It will not get encrypted and will have a special prefix added.
	LOG_API void CommitLineMetadata(std::string line);

	// Reset all data for this logger.
	LOG_API void Reset();


protected:
	std::string filePath;
	std::ofstream logFile;
	std::vector<std::string> logDataInMemory;
	std::vector<std::string> columnNames;
	bool setFilePathSuccessful = false;
	bool saveToFileSuccessful = false;
	bool encryptData = false;

	std::vector<std::shared_ptr<LogObserver>> logObservers;

	// If encrypt_data is set to true, each line logged via
	// WriteHeaderLine() or LogDataPoint(..) will be encrypted.
	LOG_API void SetEncryptOption(const bool encrypt_data);


private:
	std::string AppendNewLineIfNecessary(std::string line);
	void WriteLineToFile(std::string line);
	void WriteEncryptedLineToFile(std::string line);
	void SaveMemoryLogToFileHelper(const std::string& file_path, bool encrypt);

};


