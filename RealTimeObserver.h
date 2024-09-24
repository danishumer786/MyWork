#pragma once
#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"
#include <map>

class RealTimeObserver : public LogObserver {
public:
    // Constructor that accepts three separate plots for current, voltage, and temperature
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* currentPlot, GraphPlotting* voltagePlot, GraphPlotting* tempPlot)
        : textCtrl_(textCtrl), currentPlot_(currentPlot), voltagePlot_(voltagePlot), tempPlot_(tempPlot) {}

    // Method to handle new data points
    void onDataPointLogged(std::map<std::string, std::string> data) override {
        textCtrl_->AppendText("Received Data:\n");
        wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

        // Vectors to hold current, voltage, and temperature data
        std::vector<float> currents;
        std::vector<float> voltages;
        std::vector<float> temperatures;
        std::vector<std::string> currentLabels;
        std::vector<std::string> voltageLabels;
        std::vector<std::string> tempLabels;

        bool currentDataFound = false;
        bool voltageDataFound = false;
        bool tempDataFound = false;

        // Iterate through the received data
        for (const auto& entry : data) {
            std::string logEntry = entry.first + ": " + entry.second + "\n";
            textCtrl_->AppendText(logEntry);  // Log the received data

            try {
                // Handle current values
                if (entry.first.find("TecCurrent-") != std::string::npos) {
                    float currentValue = std::stof(entry.second);
                    std::string label = entry.first.substr(entry.first.find("-") + 1);
                    currents.push_back(currentValue);
                    currentLabels.push_back(label);
                    currentDataFound = true;
                }
                // Handle voltage values
                else if (entry.first.find("TecVoltage-") != std::string::npos) {
                    float voltageValue = std::stof(entry.second);
                    std::string label = entry.first.substr(entry.first.find("-") + 1);
                    voltages.push_back(voltageValue);
                    voltageLabels.push_back(label);
                    voltageDataFound = true;
                }
                // Handle temperature values
                else if (entry.first.find("ActualTemp-") != std::string::npos) {
                    float tempValue = std::stof(entry.second);
                    std::string label = entry.first.substr(entry.first.find("-") + 1);
                    temperatures.push_back(tempValue);
                    tempLabels.push_back(label);
                    tempDataFound = true;
                }
            }
            catch (const std::exception& e) {
                wxLogError("Error converting data: %s", e.what());
            }
        }

        // Pass data to the respective plots
        if (currentDataFound) {
            currentPlot_->AddCurrentDataPoint(currents, currentLabels, currentTime);
        }
        if (voltageDataFound) {
            voltagePlot_->AddVoltageDataPoint(voltages, voltageLabels, currentTime);
        }
        if (tempDataFound) {
            tempPlot_->AddTemperatureDataPoint(temperatures, tempLabels, currentTime);
        }
    }

private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* currentPlot_;
    GraphPlotting* voltagePlot_;
    GraphPlotting* tempPlot_;
};
