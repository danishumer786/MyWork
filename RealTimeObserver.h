#pragma once

#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"

class RealTimeObserver : public LogObserver {
public:
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* graphPlot)
        : textCtrl_(textCtrl), graphPlot_(graphPlot) {}

    void onDataPointLogged(std::map<std::string, std::string> data) override {
        textCtrl_->AppendText("Received Data:\n");
        wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

        // Vectors to hold currents, voltages, and their TEC labels
        std::vector<float> currents;
        std::vector<float> voltages;
        std::vector<std::string> currentLabels;
        std::vector<std::string> voltageLabels;

        bool currentDataFound = false;
        bool voltageDataFound = false;

        // Parse the map and extract the data
        for (const auto& entry : data) {
            std::string logEntry = entry.first + ": " + entry.second + "\n";
            textCtrl_->AppendText(logEntry);  // Log the received data

            try {
                // Check if the entry is a current or voltage and extract TEC label
                if (entry.first.find("TecCurrent-") != std::string::npos) {
                    float currentValue = std::stof(entry.second);
                    std::string label = entry.first.substr(entry.first.find("-") + 1);  // Extract label, e.g., "SHG"
                  

                    currents.push_back(currentValue);
                    currentLabels.push_back(label);  // Store the TEC label for this current
                    currentDataFound = true;

                }
                else if (entry.first.find("TecVoltage-") != std::string::npos) {
                    float voltageValue = std::stof(entry.second);
                    std::string label = entry.first.substr(entry.first.find("-") + 1);  // Extract label, e.g., "SHG"

                    voltages.push_back(voltageValue);
                    voltageLabels.push_back(label);  // Store the TEC label for this voltage
                    voltageDataFound = true;
                }
            }
            catch (const std::exception& e) {
                wxLogError("Error converting current/voltage: %s", e.what());
            }
        }

        // Pass the data along with TEC labels to the GraphPlotting object
        if (currentDataFound && voltageDataFound) {
            graphPlot_->AddDataPoint(currents, voltages, currentLabels, voltageLabels, currentTime);
        }
        else if (currentDataFound) {
            graphPlot_->AddCurrentDataPoint(currents, currentLabels, currentTime);
        }
        else if (voltageDataFound) {
            graphPlot_->AddVoltageDataPoint(voltages, voltageLabels, currentTime);
        }
    }


private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* graphPlot_;
};
