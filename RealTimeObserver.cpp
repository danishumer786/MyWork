#include "RealTimeObserver.h"
#include <wx/datetime.h>
#include <wx/log.h>

void RealTimeObserver::onDataPointLogged(std::map<std::string, std::string> data) {
    textCtrl_->AppendText("Received Data:\n");
    wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

    std::vector<std::string> alarms; 

    
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
    bool alarmDataFound = false;

    for (const auto& entry : data) {
        std::string logEntry = entry.first + ": " + entry.second + "\n";
        textCtrl_->AppendText(logEntry);

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
                diodeCurrents.push_back(diodeCurrentValue);
                diodeCurrentLabels.push_back(label);
                diodeCurrentDataFound = true;
            }
            // Handle Power readings
            else if (entry.first.find("PowerMonitor-") != std::string::npos) {
                float powerValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                powerReadings.push_back(powerValue);
                powerLabels.push_back(label);
                powerDataFound = true;
            }
            // Handle Sensor readings (Flow/Humidity)
            else if (entry.first.find("Flow") != std::string::npos || entry.first.find("Humidity-") != std::string::npos) {
                float sensorValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                sensorReadings.push_back(sensorValue);
                sensorLabels.push_back(label);
                sensorDataFound = true;
            } 

            // Handle Alarms
           else if (entry.first.find("Alarms") != std::string::npos) {
                if (!entry.second.empty()) {
                    wxString newAlarmMessage = wxString::FromUTF8(entry.second); 
                    wxString newAlarmTime = currentTime;  
                    
                    if (newAlarmMessage != lastAlarmMessage_ && newAlarmTime != lastAlarmTime_) {
                        alarms.push_back(entry.second); 
                        alarmDataFound = true;
                        alarmMessage_ = newAlarmMessage;  
                        alarmTime_ = newAlarmTime;  
                        alarmTriggered_ = true;
                        lastAlarmMessage_ = newAlarmMessage;
                        lastAlarmTime_ = newAlarmTime;
                    }
                }
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

    // Pass data to the Power plot
    if (powerDataFound && powerPlot_) {
        powerPlot_->AddPowerDataPoint(powerReadings, powerLabels, currentTime);
    }

    // Pass data to the Sensor plot
    if (sensorDataFound && sensorPlot_) {
        sensorPlot_->AddSensorDataPoint(sensorReadings, sensorLabels, currentTime);
    }

    
    if (alarmDataFound) {
        alarmTriggered_ = true;  

        for (const std::string& alarmMessage : alarms) {
            wxString newAlarmText = wxString::Format("Alarm triggered at %s: %s\n", currentTime, alarmMessage);
            textCtrl_->AppendText(newAlarmText);
            if (alarmTextCtrl_) {
                alarmTextCtrl_->AppendText(newAlarmText);
            }
        }

        if (currentPlot_) currentPlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);
        if (voltagePlot_) voltagePlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);
        if (tempPlot_) tempPlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);
        if (diodePlot_) diodePlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);
        if (powerPlot_) powerPlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);
        if (sensorPlot_) sensorPlot_->SetAlarmTriggered(true, wxString::FromUTF8(alarmMessage_), currentTime);

           
        
    }

    

}
    
    

