#include "RealTimeObserver.h"
#include <wx/datetime.h>
#include <wx/log.h>

void RealTimeObserver::onDataPointLogged(std::map<std::string, std::string> data) {
    bool visibilityUpdated = false;
    printf("i received the data");
    textCtrl_->AppendText("Received Data:\n");
    wxString currentTime = wxDateTime::Now().Format("%H:%M:%S");

    

    for (const auto& entry : data) {
        std::string logEntry = entry.first + ": " + entry.second + "\n";
        //wxLogMessage("Entry Key: %s, Entry Value: %s", entry.first.c_str(), entry.second.c_str());
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
            //wxLogMessage("PowerMonitor entry detected: key = %s, value = %s",entry.first.c_str(), entry.second.c_str());
                float powerValue = std::stof(entry.second);
                std::string label = entry.first.substr(entry.first.find("-") + 1);
                powerReadings.push_back(powerValue);

                powerLabels.push_back(label);

                powerDataFound = true;
                //wxLogMessage("Power value: %.2f, Label: %s", powerValue, label.c_str());

            }
            // Handle Sensor readings (Flow/Humidity)
            // Handle Sensor readings (Flow/Humidity)
            else if (entry.first.find("Flow") != std::string::npos || entry.first.find("Humidity-") != std::string::npos) {
                float sensorValue = std::stof(entry.second);
                std::string label;

                // Handle "Flow" and "Humidity-<label>" cases separately
                if (entry.first.find("Flow") != std::string::npos) {
                    label = "Flow";  // Directly assign "Flow" as the label
                }
                else if (entry.first.find("Humidity-") != std::string::npos) {
                    label = entry.first.substr(entry.first.find("-") + 1);  // Extract label after "-"
                }

                sensorReadings.push_back(sensorValue);
                sensorLabels.push_back(label);
                sensorDataFound = true;

                //// Log messages for debugging the flow of data
                ////wxLogMessage("Sensor Value: %.2f", sensorValue);
                ////wxLogMessage("Current sensorReadings size: %zu", sensorReadings.size());
                //for (size_t i = 0; i < sensorReadings.size(); ++i) {
                //    wxLogMessage("sensorReadings[%zu]: %.2f", i, sensorReadings[i]);
                //}
                //wxLogMessage("Current sensorLabels size: %zu", sensorLabels.size());
                //for (size_t i = 0; i < sensorLabels.size(); ++i) {
                //    wxLogMessage("sensorLabels[%zu]: %s", i, sensorLabels[i].c_str());
                //}
                //wxLogMessage("Sensor Data Found: %s", sensorDataFound ? "true" : "false");
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




    // Ensure all parent panels are open
    if (currentDataFound&&mainTecTogglePanel_ && tecContainer_&& currentPanel_ && currentToggleButton_) {
        if (!mainTecTogglePanel_->IsShown()&& !tecContainer_->IsShown()&& !currentPanel_->IsShown())
        mainTecTogglePanel_->Show();
        tecContainer_->IsShown();
        tecPanelToggleButton_->SetBitmap(collapseBitmap);
        tecContainer_->Show();
        currentPanel_->IsShown();
        currentPanel_->Show();
        currentToggleButton_->SetBitmap(collapseBitmap);
        updateLayout_();


        // Append data to the plot
        if (currentPlot_) {

            currentPlot_->AddCurrentDataPoint(currents, currentLabels, currentTime);
        }



    }
    
    if (voltageDataFound && mainTecTogglePanel_ && tecContainer_ && voltagePanel_ && voltageToggleButton_) {
        if (!mainTecTogglePanel_->IsShown() && !tecContainer_->IsShown() && !voltagePanel_->IsShown())
            mainTecTogglePanel_->Show();
        tecContainer_->IsShown();
        tecPanelToggleButton_->SetBitmap(collapseBitmap);
        tecContainer_->Show();
        voltagePanel_->IsShown();
        voltagePanel_->Show();
        voltageToggleButton_->SetBitmap(collapseBitmap);
        updateLayout_();


        // Append data to the plot
        if (voltagePlot_) {

            voltagePlot_->AddCurrentDataPoint(voltages, voltageLabels, currentTime);
        }
    }
    
    if (tempDataFound && mainTecTogglePanel_ && tecContainer_ && voltagePanel_ && tempToggleButton_) {
        if (!mainTecTogglePanel_->IsShown() && !tecContainer_->IsShown() && !tempPanel_->IsShown())
            mainTecTogglePanel_->Show();
        tecContainer_->IsShown();
        tecPanelToggleButton_->SetBitmap(collapseBitmap);
        tecContainer_->Show();
        tempPanel_->IsShown();
        tempPanel_->Show();
        voltageToggleButton_->SetBitmap(collapseBitmap);
        updateLayout_();


        // Append data to the plot
        if (tempPlot_) {

            tempPlot_->AddCurrentDataPoint(temperatures, tempLabels, currentTime);
        }
    }

    // Pass data to the Diode plot
    if (diodeCurrentDataFound && diodeTogglePanel_ && diodeContentPanel_ && diodeToggleButton_) {
        if (!diodeTogglePanel_->IsShown()) {
            diodeTogglePanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        if (!diodeContentPanel_->IsShown()) {
            diodeContentPanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        diodeToggleButton_->SetBitmap(collapseBitmap);

        if (diodePlot_) {
            diodePlot_->AddDiodeCurrentDataPoint(diodeCurrents, diodeCurrentLabels, currentTime);
        }
    }

    // Pass data to the Power plot
    if (powerDataFound && powerTogglePanel_ && powerContentPanel_ && powerToggleButton_) {
        if (!powerTogglePanel_->IsShown()) {
            powerTogglePanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        if (!powerContentPanel_->IsShown()) {
            powerContentPanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        powerToggleButton_->SetBitmap(collapseBitmap);

        if (powerPlot_) {
            powerPlot_->AddPowerDataPoint(powerReadings, powerLabels, currentTime);
        }
    }

    // Pass data to the Sensor plot
    if (sensorDataFound && sensorTogglePanel_ && sensorContentPanel_ && sensorToggleButton_) {
        if (!sensorTogglePanel_->IsShown()) {
            sensorTogglePanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        if (!sensorContentPanel_->IsShown()) {
            sensorContentPanel_->Show();
            visibilityUpdated = true; // Set visibility flag
        }
        sensorToggleButton_->SetBitmap(collapseBitmap);

        if (sensorPlot_) {
            sensorPlot_->AddSensorDataPoint(sensorReadings, sensorLabels, currentTime);
        }
    }

    // Dynamically update layout if visibility changed
    if (visibilityUpdated && updateLayout_) {
        updateLayout_();
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
    // Update layout dynamically if any visibility changed
    if (visibilityUpdated && updateLayout_) {
        updateLayout_();


    }
}
    
    

