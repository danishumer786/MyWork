#include "RealTimeObserver.h"
#include <wx/datetime.h>
#include <wx/log.h>

void RealTimeObserver::onDataPointLogged(std::map<std::string, std::string> data) {
    wxLogDebug("RealTimeObserver: Received Data");
    textCtrl_->AppendText("Received Data:\n");
    wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

    // Separate data containers for TEC and Diode
    std::vector<float> currents;           // For TEC currents
    std::vector<float> voltages;           // For TEC voltages
    std::vector<float> temperatures;       // For TEC temperatures
    std::vector<float> diodeCurrents;      // For Diode currents

    std::vector<std::string> currentLabels;       // Labels for TEC currents
    std::vector<std::string> voltageLabels;       // Labels for TEC voltages
    std::vector<std::string> tempLabels;          // Labels for TEC temperatures
    std::vector<std::string> diodeCurrentLabels;  // Labels for Diode currents

    // Flags to track if any data was found
    bool currentDataFound = false;
    bool voltageDataFound = false;
    bool tempDataFound = false;
    bool diodeCurrentDataFound = false;

    // Iterate through the received data
    for (const auto& entry : data) {
        wxLogDebug("Data Entry: %s -> %s", entry.first, entry.second);
        std::string logEntry = entry.first + ": " + entry.second + "\n";
        textCtrl_->AppendText(logEntry);  // Log the received data

        try {
            // Handle TEC current values
            if (entry.first.find("TecCurrent-") != std::string::npos) {
                float currentValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                currents.push_back(currentValue);
                currentLabels.push_back(label);
                currentDataFound = true;
            }
            // Handle TEC voltage values
            else if (entry.first.find("TecVoltage-") != std::string::npos) {
                float voltageValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                voltages.push_back(voltageValue);
                voltageLabels.push_back(label);
                voltageDataFound = true;
            }
            // Handle TEC temperature values
            else if (entry.first.find("ActualTemp-") != std::string::npos) {
                float tempValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                temperatures.push_back(tempValue);
                tempLabels.push_back(label);
                tempDataFound = true;
            }
            // Handle Diode current values
            else if (entry.first.find("ActualCurrent-") != std::string::npos) {
                float diodeCurrentValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                //wxLogMessage("Diode Current Label: %s", label);
                diodeCurrents.push_back(diodeCurrentValue);
                diodeCurrentLabels.push_back(label);
                diodeCurrentDataFound = true;
            }
        }
        catch (const std::exception& e) {
            wxLogError("Error converting data: %s", e.what());
        }
    }

    // Pass data to the respective TEC plots
    if (currentDataFound && currentPlot_) {
        currentPlot_->AddCurrentDataPoint(currents, currentLabels, currentTime);
    }
    if (voltageDataFound && voltagePlot_) {
        voltagePlot_->AddVoltageDataPoint(voltages, voltageLabels, currentTime);
    }
    if (tempDataFound && tempPlot_) {
        tempPlot_->AddTemperatureDataPoint(temperatures, tempLabels, currentTime);
    }

    // Pass data to the Diode plot
    if (diodeCurrentDataFound && diodePlot_) {
        diodePlot_->AddDiodeCurrentDataPoint(diodeCurrents, diodeCurrentLabels, currentTime);
    }

}
