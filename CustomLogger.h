/**
* Custom Logger - Allows user to customize laser state log settings.
*	- Maintains own thread for continuously running log in background.
* 
* Basic Usage:
* 
*	1. Choose which laser state categories to include:
*		> customLogger.IncludeCategory(DIODE_CURRENTS);
* 
*	2. Select log output file (inherited from BaseLogger):
*		> customLogger.SetFilePath(filePath);
* 
*	3. Choose log interval:
*		> customLogger.SetTimeIntervalInSeconds(300); // 5 minutes
* 
*	4. Start logging:
*		> customLogger.Start();
* 
* @file CustomLogger.h
* @author James Butcher
* @created February, 2022
* @version 1.0
*/
#pragma once

#include "Logging/LoggerBase.h"
#include "../MainLaserController.h"
#include<string>


// Use these category enums to reference categories for inclusion or exclusion
// in the CustomLogger, or to get their names for displaying in a menu.
enum LaserStateLogCategoryEnum {
	NULL_CATEGORY,
	POWER,				// Power monitor readings
	DIODE_CURRENTS,		// Diode set currents and actual currents
	TEMPERATURES,		// Actual temperatures
	TEC_POWER,			// TECs Voltage * Current
	SENSORS,			// Flow, humidity
	PULSE_INFO,			// PRF, PEC
	MOTORS,				// Motor indexes
	ALARMS,				// Hard and soft alarms
	TEC_CURRENT,        // TEC Current
	TEC_VOLTAGE         // TEC Voltage
};

// Change the ordering of the categories here
const vector<LaserStateLogCategoryEnum> LASER_STATE_LOG_CATEGORIES{
	POWER,
	DIODE_CURRENTS,
	TEMPERATURES,
	TEC_POWER,
	SENSORS,
	PULSE_INFO,
	MOTORS,
	ALARMS,
	TEC_CURRENT,
	TEC_VOLTAGE
};


class CustomLogger : public LoggerBase {

public:
	LOG_API CustomLogger(std::shared_ptr<MainLaserControllerInterface> laser_controller, const std::string& logFilePath);
	LOG_API CustomLogger(std::shared_ptr<MainLaserControllerInterface> laser_controller);

	LOG_API void SetLogFilePath(const std::string& path);

	LOG_API std::string GetCategoryName(LaserStateLogCategoryEnum _category);

	// Include a category of laser parameters to log.
	// CustomLogger begins with no categories added.
	LOG_API void IncludeCategory(LaserStateLogCategoryEnum _category);

	// Remove a previously-included category from logging.
	LOG_API void ExcludeCategory(LaserStateLogCategoryEnum _category);

	LOG_API bool CategoryIsIncluded(LaserStateLogCategoryEnum _category) const;

	// Logger will log a data point every time interval set here.
	// Default: 10 seconds.
	LOG_API void SetTimeIntervalInSeconds(const unsigned int time_interval_in_s);
	LOG_API unsigned int GetTimeIntervalInSeconds() const;

	LOG_API void Start();
	LOG_API void Stop();
	LOG_API bool IsLogging() const;
	LOG_API void Reset();

	LOG_API unsigned int GetTotalLoggedDataPoints() const;


protected:
	class Impl;
	std::shared_ptr<Impl> impl;

	


private:
	std::string logFilePath;  // 


};

