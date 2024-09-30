#include "RealTimeObserver.h"
#include <wx/datetime.h>
#include <wx/log.h>

void RealTimeObserver::onDataPointLogged(std::map<std::string, std::string> data) {
   // wxLogMessage("RealTimeObserver: Data received with %zu entries.", data.size());
    textCtrl_->AppendText("Received Data:\n");
    wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

    // Separate data containers for TEC, Diode, and Power
    std::vector<float> currents;
    std::vector<float> voltages;
    std::vector<float> temperatures;
    std::vector<float> diodeCurrents;
    std::vector<float> powerReadings;
    std::vector<float> sensorReadings;
    std::vector<std::string> sensorLabels;

    std::vector<std::string> currentLabels;
    std::vector<std::string> voltageLabels;
    std::vector<std::string> tempLabels;
    std::vector<std::string> diodeCurrentLabels;
    std::vector<std::string> powerLabels;

    bool currentDataFound = false;
    bool voltageDataFound = false;
    bool tempDataFound = false;
    bool diodeCurrentDataFound = false;
    bool powerDataFound = false;
    bool sensorDataFound = false;

    for (const auto& entry : data) {
        //wxLogMessage("Data Entry: Key = %s, Value = %s", entry.first, entry.second);
        std::string logEntry = entry.first + ": " + entry.second + "\n";
        textCtrl_->AppendText(logEntry);

        try {
            if (entry.first.find("TecCurrent-") != std::string::npos) {
                float currentValue = std::stof(entry.second);
                //wxLogMessage("Parsed TEC current: %f", currentValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                currents.push_back(currentValue);
                currentLabels.push_back(label);
                currentDataFound = true;
            }
            else if (entry.first.find("TecVoltage-") != std::string::npos) {
                float voltageValue = std::stof(entry.second);
                //wxLogMessage("Parsed TEC voltage: %f", voltageValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                voltages.push_back(voltageValue);
                voltageLabels.push_back(label);
                voltageDataFound = true;
            }
            else if (entry.first.find("ActualTemp-") != std::string::npos) {
                float tempValue = std::stof(entry.second);
               // wxLogMessage("Parsed TEC temperature: %f", tempValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                temperatures.push_back(tempValue);
                tempLabels.push_back(label);
                tempDataFound = true;
            }
            else if (entry.first.find("ActualCurrent-") != std::string::npos) {
                float diodeCurrentValue = std::stof(entry.second);
               // wxLogMessage("Parsed diode current: %f", diodeCurrentValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                diodeCurrents.push_back(diodeCurrentValue);
                diodeCurrentLabels.push_back(label);
                diodeCurrentDataFound = true;
            }
            else if (entry.first.find("PowerMonitor-") != std::string::npos) {
                float powerValue = std::stof(entry.second);
                //wxLogMessage("Parsed power: %f", powerValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                powerReadings.push_back(powerValue);
                powerLabels.push_back(label);
                powerDataFound = true;
            }
            else if (entry.first.find("Flow") != std::string::npos || entry.first.find("Humidity-") != std::string::npos) {
                float sensorValue = std::stof(entry.second);
               // wxLogMessage("Parsed sensor reading: %f", sensorValue);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                sensorReadings.push_back(sensorValue);
                sensorLabels.push_back(label);
                sensorDataFound = true;
            }
        }
        catch (const std::exception& e) {
            wxLogError("Error converting data: %s", e.what());
        }
    }

    if (currentDataFound && currentPlot_) {
        wxLogMessage("Sending TEC current data to plot: %zu data points.", currents.size());
        currentPlot_->AddCurrentDataPoint(currents, currentLabels, currentTime);
    }
    if (voltageDataFound && voltagePlot_) {
        wxLogMessage("Sending TEC voltage data to plot: %zu data points.", voltages.size());
        voltagePlot_->AddVoltageDataPoint(voltages, voltageLabels, currentTime);
    }
    if (tempDataFound && tempPlot_) {
        wxLogMessage("Sending TEC temperature data to plot: %zu data points.", temperatures.size());
        tempPlot_->AddTemperatureDataPoint(temperatures, tempLabels, currentTime);
    }

    if (diodeCurrentDataFound && diodePlot_) {
        wxLogMessage("Sending Diode current data to plot: %zu data points.", diodeCurrents.size());
        diodePlot_->AddDiodeCurrentDataPoint(diodeCurrents, diodeCurrentLabels, currentTime);
    }

    if (powerDataFound && powerPlot_) {
        wxLogMessage("Sending Power data to plot: %zu data points.", powerReadings.size());
        powerPlot_->AddPowerDataPoint(powerReadings, powerLabels, currentTime);
    }

    if (sensorDataFound && sensorPlot_) {
        //wxLogMessage("Sending Sensor data to plot: %zu data points.", sensorReadings.size());
        sensorPlot_->AddSensorDataPoint(sensorReadings, sensorLabels, currentTime);
    }
}
