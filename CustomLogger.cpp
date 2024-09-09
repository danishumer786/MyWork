// TO ADD A NEW CATEGORY:
//	1. Add a new category enum value to LaserStateLogCategoryEnum (in header file)
//	2. Add the new enum to LASER_STATE_LOG_CATEGORIES (in header file)
//	3. Derive a new class from LaserStateLogCategory, overriding the virtual methods (this file)
//  4. Instantiate the new class in the CustomLogger::Impl::Impl constructor below by adding a new row
//		e.g., mapEnumToCategory[NEW_CATEGORY] = make_shared<LaserStateLogCategory_NewCategory>(laserController);

#include "CustomLogger.h"
//#include "LaserGUI/GraphWindow.h"



using namespace std;

const unsigned int DEFAULT_LOG_TIME_INTERVAL_IN_S = 60;


// Base class for custom log categories.
// Manages:
//	- category name (e.g., "Temperatures")
//	- laser parameter names contained in the category 
//		(e.g., "ActualTemp-SHG", "ActualTemp-THG", ...), for use in column headers
//	- getting the values of the parameters, for use as the actual data values
//	- switching the category ON/OFF for being included in the log
// 
// You can arbitrarily control which laser parameters are included in a category
// by choosing the names returned by GetColumnNames() and the values returned
// by GetValues(). The only requirement is that the number of items returned in each
// must match, otherwise logging the data point will fail.
//
// Create a new category by deriving from this class and overriding the
// 3 virtual methods: GetName(), GetColumnNames(), and GetValues().
// See the child classes below for examples.
class LaserStateLogCategory {

protected:
	shared_ptr<MainLaserControllerInterface> lc;
	bool isIncluded = false;

public:
	LaserStateLogCategory(shared_ptr<MainLaserControllerInterface> laser_controller) : lc(laser_controller) {}

	// Return the name of the logging category.
	// E.g., "Temperatures".
	virtual string GetName() const = 0;

	// Get the string values from this category to be put in the header line.
	// E.g., { "SHG", "THG", "LDD" }
	virtual vector<string> GetColumnNames() const = 0;

	// Get the string values from this category to be put in the log.
	// E.g., { "49.5", "51.3", "27.8" }
	virtual vector<string> GetValues() const = 0;

	// Include this category in the log.
	void Include() { isIncluded = true; };

	// Exclude this category from the log.
	void Exclude() { isIncluded = false; };

	// Indicates whether this category will be included in the log.
	bool IsIncluded() const { return isIncluded; };
};


// ----------------------------------------------------------------------------
// Power
class LaserStateLogCategory_Power : public LaserStateLogCategory {
public:
	LaserStateLogCategory_Power(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Power"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX = "PowerMonitor-";
		for (int id : lc->GetPowerMonitorIDs()) {
			const string LABEL = lc->GetPowerMonitorLabel(id);
			columnNames.push_back(PREFIX + LABEL);
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshPowerMonitorReadings();
		for (int id : lc->GetPowerMonitorIDs())
			values.push_back(to_string_with_precision(lc->GetPowerMonitorReadingInWatts(id), 2));
		return values;
	}
};

// ----------------------------------------------------------------------------
// Diode Currents
class LaserStateLogCategory_DiodeCurrents : public LaserStateLogCategory {
public:
	LaserStateLogCategory_DiodeCurrents(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Diode Currents"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX_SET_CURRENT = "SetCurrent-";
		const string PREFIX_ACTUAL_CURRENT = "ActualCurrent-";
		for (int id : lc->GetLddIds()) {
			const string LABEL = lc->GetLDDLabel(id);
			columnNames.push_back(PREFIX_SET_CURRENT + LABEL);
			columnNames.push_back(PREFIX_ACTUAL_CURRENT + LABEL);
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshLDDReadings();
		for (int id : lc->GetLddIds()) {
			values.push_back(to_string_with_precision(lc->GetLDDSetCurrent(id), 2));
			values.push_back(to_string_with_precision(lc->GetLDDActualCurrent(id), 2));
		}
		return values;
	}
};

// ----------------------------------------------------------------------------
// Temperatures
class LaserStateLogCategory_Temperatures : public LaserStateLogCategory {
public:
	LaserStateLogCategory_Temperatures(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Temperatures"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX_SET = "SetTemp-";
		const string PREFIX_ACTUAL = "ActualTemp-";
		for (int id : lc->GetTemperatureControlIDs()) {
			const string LABEL = lc->GetTemperatureControlLabel(id);
			if (lc->TemperatureControlIsSettable(id)) {
				columnNames.push_back(PREFIX_SET + LABEL);
			}
			columnNames.push_back(PREFIX_ACTUAL + LABEL);
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshTemperatureReadings();
		for (int id : lc->GetTemperatureControlIDs()) {
			if (lc->TemperatureControlIsSettable(id)) {
				values.push_back(to_string_with_precision(lc->GetSetTemperature(id), 2));
			}
			values.push_back(to_string_with_precision(lc->GetActualTemperature(id), 2));
		}
		return values;
	}
};
// ----------------------------------------------------------------------------
// TEC Voltage Class
class LaserStateLogCategory_TECVoltage : public LaserStateLogCategory {
public:
	LaserStateLogCategory_TECVoltage(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "TEC Voltage"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX = "TecVoltage-";
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				const string LABEL = lc->GetTemperatureControlLabel(id);
				columnNames.push_back(PREFIX + LABEL);
			}
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshTECVoltageAndCurrentReadings();
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				float voltage = lc->GetTECVoltage(id);
				values.push_back(to_string_with_precision(voltage, 3));
			}
		}
		return values;
	}
};

// ----------------------------------------------------------------------------
// TEC Current Class
class LaserStateLogCategory_TECCurrent : public LaserStateLogCategory {
public:
	LaserStateLogCategory_TECCurrent(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "TEC Current"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX = "TecCurrent-";
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				const string LABEL = lc->GetTemperatureControlLabel(id);
				columnNames.push_back(PREFIX + LABEL);
			}
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshTECVoltageAndCurrentReadings();
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				float current = lc->GetTECCurrent(id);
				values.push_back(to_string_with_precision(current, 3));
			}
		}
		return values;
	}
};
// ----------------------------------------------------------------------------
// TEC Power - |Current * Voltage|
class LaserStateLogCategory_TECPower : public LaserStateLogCategory {
public:
	LaserStateLogCategory_TECPower(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "TEC Power"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX = "TecPower-";
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				const string LABEL = lc->GetTemperatureControlLabel(id);
				columnNames.push_back(PREFIX + LABEL);
			}
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		lc->RefreshTECVoltageAndCurrentReadings();
		for (int id : lc->GetTemperatureControlIDs()) {
			if (!lc->TemperatureControlIsThermistorOnly(id)) {
				float current = lc->GetTECCurrent(id);
				float voltage = lc->GetTECVoltage(id);
				float power = abs(current * voltage);
				values.push_back(to_string_with_precision(power, 3));
			}
		}
		return values;
	}
};

// ----------------------------------------------------------------------------
// Sensors
class LaserStateLogCategory_Sensors : public LaserStateLogCategory {
public:
	LaserStateLogCategory_Sensors(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Sensors"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;

		// Chiller flow
		if (lc->ChillerFlowIsEnabledForUse())
			columnNames.push_back("Flow");

		// Humidity
		const string PREFIX = "Humidity-";
		for (int id : lc->GetHumidityIds()) {
			const string LABEL = lc->GetHumidityLabel(id);
			columnNames.push_back(PREFIX + LABEL);
		}

		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;

		// Chiller flow
		if (lc->ChillerFlowIsEnabledForUse()) {
			lc->RefreshFlowReadings();
			values.push_back(to_string_with_precision(lc->GetChillerFlowReading(), 1));
		}

		// Humidity
		auto humidityIds = lc->GetHumidityIds();
		if (humidityIds.size() != 0) {
			lc->RefreshHumidityReadings();
			for (int id : humidityIds) {
				values.push_back(to_string(lc->GetHumidityReading(id)));
			}
		}

		return values;
	}
};

// ----------------------------------------------------------------------------
// Pulse Info
class LaserStateLogCategory_PulseInfo : public LaserStateLogCategory {
public:
	LaserStateLogCategory_PulseInfo(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Pulse Info"; }

	vector<string> GetColumnNames() const override {
		return {
			"PRF",
			"PEC"
		};
	}

	vector<string> GetValues() const override {
		vector<string> values;
		values.push_back(to_string(lc->GetPRF()));
		values.push_back(to_string_with_precision(lc->GetPEC(), 2));
		return values;
	}
};

// ----------------------------------------------------------------------------
// Motors
class LaserStateLogCategory_Motors : public LaserStateLogCategory {
public:
	LaserStateLogCategory_Motors(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Motors"; }

	vector<string> GetColumnNames() const override {
		vector<string> columnNames;
		const string PREFIX = "MotorIndex-";
		for (int id : lc->GetMotorIDs()) {
			const string LABEL = lc->GetMotorLabel(id);
			columnNames.push_back(PREFIX + LABEL);
		}
		return columnNames;
	}

	vector<string> GetValues() const override {
		vector<string> values;
		auto motorIds = lc->GetMotorIDs();
		if (motorIds.size() != 0) {
			lc->RefreshMotorReadings();
			for (int id : motorIds)
				values.push_back(to_string(lc->GetMotorIndex(id)));
		}
		return values;
	}
};

// ----------------------------------------------------------------------------
// Alarms
class LaserStateLogCategory_Alarms : public LaserStateLogCategory {
public:
	LaserStateLogCategory_Alarms(
		shared_ptr<MainLaserControllerInterface> laser_controller) :
		LaserStateLogCategory(laser_controller) {}

	string GetName() const override { return "Alarms"; }

	vector<string> GetColumnNames() const override { return { "Alarms" }; };

	vector<string> GetValues() const override {
		// Returns a single value: either an empty string or a string of one or more alarms
		vector<string> values;
		lc->RefreshVitalStatusReadings();
		if (lc->HasSoftFault() or lc->HasHardFault()) {
			string faultsMessage = "";
			for (string& fault : lc->GetAllCurrentFaults())
				faultsMessage += bracketize(fault);
			values.push_back(faultsMessage);
		}
		else
			values.push_back("");
		return values;
	}
};




// Logger implementation - hidden from header file
class CustomLogger::Impl {

private:
	void StepLogLaserStateThread() {

		chrono::steady_clock::time_point startTime = chrono::high_resolution_clock::now();
		unsigned int totalLoggedDataPointsInThisThread = 0;

		while (isLogging and lc->IsConnected()) {

			chrono::steady_clock::time_point currentTime = chrono::high_resolution_clock::now();
			auto durationInSeconds = chrono::duration_cast<chrono::seconds>(currentTime - startTime);
			unsigned int timeElapsedInSeconds = unsigned int(durationInSeconds.count());

			if (timeElapsedInSeconds >= (timeIntervalInS * totalLoggedDataPointsInThisThread)) {
				LogDataPoint();
				totalLoggedDataPointsInThisThread++;
			}

			this_thread::sleep_for(chrono::seconds(1));
		}

	}

	void LogDataPoint() {
		if (!lc->IsConnected())
			return;
		vector<string> values;
		for (auto& category : categories) {
			if (category->IsIncluded()) {

				if (!lc->IsConnected() or lc->IsResetting() or lc->IsUpdating())
					return;

				vector<string> categoryValues = category->GetValues();
				values.insert(values.end(), categoryValues.begin(), categoryValues.end());
			}
		}
		l.LogDataPoint(values);
		totalLoggedDataPoints++;
	}

public:
	CustomLogger::Impl(shared_ptr<MainLaserControllerInterface> laser_controller, CustomLogger& _l)
		: lc(laser_controller), l(_l) {
		mapEnumToCategory[POWER] = make_shared<LaserStateLogCategory_Power>(lc);
		mapEnumToCategory[DIODE_CURRENTS] = make_shared<LaserStateLogCategory_DiodeCurrents>(lc);
		mapEnumToCategory[TEMPERATURES] = make_shared<LaserStateLogCategory_Temperatures>(lc);
		mapEnumToCategory[TEC_POWER] = make_shared<LaserStateLogCategory_TECPower>(lc);
		mapEnumToCategory[SENSORS] = make_shared<LaserStateLogCategory_Sensors>(lc);
		mapEnumToCategory[PULSE_INFO] = make_shared<LaserStateLogCategory_PulseInfo>(lc);
		mapEnumToCategory[MOTORS] = make_shared<LaserStateLogCategory_Motors>(lc);
		mapEnumToCategory[ALARMS] = make_shared<LaserStateLogCategory_Alarms>(lc);
		mapEnumToCategory[TEC_CURRENT] = make_shared<LaserStateLogCategory_TECCurrent>(lc);  // For current
		mapEnumToCategory[TEC_VOLTAGE] = make_shared<LaserStateLogCategory_TECVoltage>(lc);  // For voltage

		for (auto categoryEnum : LASER_STATE_LOG_CATEGORIES)
			categories.push_back(mapEnumToCategory.at(categoryEnum));
	}

	~Impl() {
		if (loggingThread != nullptr) {
			loggingThread->join();
		}
	}

	CustomLogger& l;
	std::string logFilePath;  // Add this line
	shared_ptr<MainLaserControllerInterface> lc;
	vector<shared_ptr<LaserStateLogCategory>> categories;
	map<LaserStateLogCategoryEnum, shared_ptr<LaserStateLogCategory>> mapEnumToCategory;
	unsigned int timeIntervalInS = DEFAULT_LOG_TIME_INTERVAL_IN_S;
	unsigned int totalLoggedDataPoints = 0;
	shared_ptr<thread> loggingThread = nullptr;
	bool isLogging = false;

	void InitLoggingThread() {
		if (loggingThread != nullptr) {
			loggingThread->join();
			loggingThread.reset();
		}
		loggingThread = make_shared<thread>(&CustomLogger::Impl::StepLogLaserStateThread, this);
	}
};

CustomLogger::CustomLogger(shared_ptr<MainLaserControllerInterface> laser_controller) :
	impl(make_shared<CustomLogger::Impl>(laser_controller, *this)) {
}

CustomLogger::CustomLogger(shared_ptr<MainLaserControllerInterface> laser_controller, const std::string& logFilePath) :
	impl(make_shared<CustomLogger::Impl>(laser_controller, *this)) {
	SetLogFilePath(logFilePath); // Use the new function to set the log file path
}

void CustomLogger::SetLogFilePath(const std::string& path) {
	this->SetFilePath(path);
}

string CustomLogger::GetCategoryName(LaserStateLogCategoryEnum _category) {
	return impl->mapEnumToCategory.at(_category)->GetName();
}

void CustomLogger::IncludeCategory(LaserStateLogCategoryEnum _category) {
	auto& category = impl->mapEnumToCategory.at(_category);
	category->Include();
}

void CustomLogger::ExcludeCategory(LaserStateLogCategoryEnum _category) {
	auto& category = impl->mapEnumToCategory.at(_category);
	category->Exclude();
}

bool CustomLogger::CategoryIsIncluded(LaserStateLogCategoryEnum _category) const {
	return impl->mapEnumToCategory.at(_category)->IsIncluded();
}

void CustomLogger::SetTimeIntervalInSeconds(const unsigned int time_interval_in_s) {
	if (time_interval_in_s > 0 and time_interval_in_s <= 999999)
		impl->timeIntervalInS = time_interval_in_s;
}

unsigned int CustomLogger::GetTimeIntervalInSeconds() const {
	return impl->timeIntervalInS;
}

void CustomLogger::Start() {
	// Add columns for included categories
	columnNames.clear();
	AddColumn("Date");
	AddColumn("Time");
	for (auto& category : impl->categories) {
		if (category->IsIncluded()) {
			for (const string& columnName : category->GetColumnNames())
				AddColumn(columnName);
		}
	}
	if (GetTotalLoggedDataPoints() == 0)
		WriteHeaderLine();

	impl->InitLoggingThread();
	impl->isLogging = true;
}

void CustomLogger::Stop() {
	impl->isLogging = false;
}

bool CustomLogger::IsLogging() const {
	return impl->isLogging;
}

void CustomLogger::Reset() {
	impl->totalLoggedDataPoints = 0;

	// Clear file contents
	logFile.open(impl->logFilePath, ofstream::out | ofstream::trunc); // Update to use logFilePath
	logFile.close();
	logDataInMemory.clear();

	if (IsLogging())
		WriteHeaderLine();
}

unsigned int CustomLogger::GetTotalLoggedDataPoints() const {
	return impl->totalLoggedDataPoints;
}
