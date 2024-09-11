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
                    float currentValue = std::stof(entry.second);

                    // Debugging: Add a sanity check for high values
                    if (currentValue > 1000) {  // Adjust threshold as per your needs
                        wxLogError("Warning: Unusually high current value: %f", currentValue);
                    }

                    currents.push_back(currentValue);  // Store the current value
                    currentDataFound = true;
                }
                else if (entry.first.find("TecVoltage-") != std::string::npos) {
                    float voltageValue = std::stof(entry.second);

                    // Debugging: Add a sanity check for high values
                    if (voltageValue > 1000 || voltageValue < -1000) {  // Adjust threshold as per your needs
                        wxLogError("Warning: Unusually high or low voltage value: %f", voltageValue);
                    }

                    voltages.push_back(voltageValue);  // Store the voltage value
                    voltageDataFound = true;
                }
            }
            catch (const std::exception& e) {
                wxLogError("Error converting current/voltage: %s", e.what());
            }
        }

        // Decide which data to pass to the graphPlot_
        if (currentDataFound && voltageDataFound) {
            graphPlot_->AddDataPoint(currents, voltages, currentTime);  // Pass both currents and voltages
        }
        else if (currentDataFound) {
            graphPlot_->AddCurrentDataPoint(currents, currentTime);  // Pass only currents
        }
        else if (voltageDataFound) {
            graphPlot_->AddVoltageDataPoint(voltages, currentTime);  // Pass only voltages
        }
    }



private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* graphPlot_;
};
