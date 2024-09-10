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

        // Vectors to hold currents and voltages (if available)
        std::vector<float> currents;
        std::vector<float> voltages;

        bool currentDataFound = false;
        bool voltageDataFound = false;

        // Parse the map and extract the data
        for (const auto& entry : data) {
            std::string logEntry = entry.first + ": " + entry.second + "\n";
            textCtrl_->AppendText(logEntry);  // Log the received data

            try {
                // Check if the entry is a current or voltage
                if (entry.first.find("TecCurrent-") != std::string::npos) {
                    currents.push_back(std::stof(entry.second));  // Store the current value
                    currentDataFound = true;
                }
                else if (entry.first.find("TecVoltage-") != std::string::npos) {
                    voltages.push_back(std::stof(entry.second));  // Store the voltage value
                    voltageDataFound = true;
                }
            }
            catch (const std::exception& e) {
                wxLogError("Error converting current/voltage: %s", e.what());
            }
        }

        // If either currents or voltages are found, pass them to GraphPlotting
        if (currentDataFound || voltageDataFound) {
            graphPlot_->AddDataPoint(currents, voltages, currentTime);  // Pass only the available data
        }
    }

private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* graphPlot_;
};
