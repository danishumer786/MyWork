#pragma once

#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"

class RealTimeObserver : public LogObserver {
public:
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* graphPlot, const std::map<std::string, wxCheckBox*>& tecCheckboxes)
        : textCtrl_(textCtrl), graphPlot_(graphPlot), tecCheckboxes_(tecCheckboxes) {}
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

           // wxLogMessage("Received data - Key: %s, Value: %s", entry.first, entry.second);

            try {
                // Check if the entry is a current or voltage
                if (entry.first.find("TecCurrent-") != std::string::npos || entry.first.find("TecVoltage-") != std::string::npos) {
                    // Extract TEC label from the entry key, e.g., "SHG" from "TecCurrent-SHG"
                    std::string label = entry.first.substr(entry.first.find("-") + 1);

                    // Check if the corresponding checkbox for this TEC is checked
                    if (tecCheckboxes_.at(label)->IsChecked()) {
                        // If it's checked, process the data
                        if (entry.first.find("TecCurrent-") != std::string::npos) {
                            currents.push_back(std::stof(entry.second));
                            currentDataFound = true;
                        }
                        else if (entry.first.find("TecVoltage-") != std::string::npos) {
                            voltages.push_back(std::stof(entry.second));
                            voltageDataFound = true;
                        }
                    }
                    else {
                        // If it's unchecked, skip processing for this TEC
                        //wxLogMessage("TEC %s is unchecked, skipping data.", label);
                    }
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
    std::map<std::string, wxCheckBox*> tecCheckboxes_;
};
