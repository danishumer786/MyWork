// James Butcher
// 8/22/24

#pragma once

#include <map>
#include <string>


// Log Observer - Interface for classes that need to be notified when
//	a Logger object logs a data point. (See LoggerBase::LogDataPoint(..)).
// 
// Receives log data point data as a map of <column name> : <value> pairs
//	from a Log Notifier.
//
// Example: If observing a custom logger logging power and temperature,
//	and there is one power monitor: "GRN" and two temperature modules:
//	"SHG" and "LD", then the data map passed to the LogObserver at each
//	data point would be:
//	{ 
//		{ "Date" : "08-22-2024" },
//		{ "Time" : "11:58:37" },
//		{ "Time" : "11:58:37" },
//		{ "PowerMonitor-GRN" : "15.5" },
//		{ "SetTemp-SHG" : "50.0" },
//		{ "ActualTemp-SHG" : "50.22" },
//		{ "SetTemp-LD" : "35.0" },
//		{ "ActualTemp-LD" : "22.3" },
//	}
class LogObserver {

public:
	virtual void onDataPointLogged(std::map<std::string, std::string> data) = 0;

};

