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

        float tec1Current = 0.0f, tec2Current = 0.0f;
        float tec1Voltage = 0.0f, tec2Voltage = 0.0f;
        bool tec1CurrentFound = false, tec2CurrentFound = false;
        bool tec1VoltageFound = false, tec2VoltageFound = false;

        for (const auto& entry : data) {
            std::string logEntry = entry.first + ": " + entry.second + "\n";
            textCtrl_->AppendText(logEntry);

            try {
                // Check for TEC1 current
                if (entry.first.find("TecCurrent-SHG") != std::string::npos) {
                    tec1Current = std::stof(entry.second);
                    tec1CurrentFound = true;
                }
                // Check for TEC2 current
                else if (entry.first.find("TecCurrent-THG") != std::string::npos) {
                    tec2Current = std::stof(entry.second);
                    tec2CurrentFound = true;
                }
                // Check for TEC1 voltage
                else if (entry.first.find("TecVoltage-SHG") != std::string::npos) {
                    tec1Voltage = std::stof(entry.second);
                    tec1VoltageFound = true;
                }
                // Check for TEC2 voltage
                else if (entry.first.find("TecVoltage-THG") != std::string::npos) {
                    tec2Voltage = std::stof(entry.second);
                    tec2VoltageFound = true;
                }
            }
            catch (const std::exception& e) {
                wxLogError("Error converting current/voltage: %s", e.what());
            }
        }

        // Pass data to GraphPlotting if all values are found
        if (tec1CurrentFound && tec2CurrentFound && tec1VoltageFound && tec2VoltageFound) {
            graphPlot_->AddDataPoint(tec1Current, tec2Current, tec1Voltage, tec2Voltage, currentTime);  // Pass all data and time to GraphPlotting
        }
    }

private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* graphPlot_;
};
