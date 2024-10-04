#include "GraphPlotting.h"
#include "wx/dcbuffer.h"
#include <algorithm>
#include <limits>

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
EVT_PAINT(GraphPlotting::paintEvent)
EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, const std::vector<wxCheckBox*>& checkboxes)
    : wxPanel(parent, winid, pos, size),
    checkboxes_(checkboxes), currentMax_(std::numeric_limits<float>::lowest()),
    currentMin_(std::numeric_limits<float>::max()),
    voltageMax_(std::numeric_limits<float>::lowest()),
    voltageMin_(std::numeric_limits<float>::max()) {  

    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents,
    const std::vector<std::string>& currentLabels, const wxString& time) {

    currentData_.push_back(currents);

    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label); 
    }
    timeData_.push_back(time);

    for (const auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddVoltageDataPoint(const std::vector<float>& voltages,
    const std::vector<std::string>& voltageLabels,
    const wxString& time) {
    // Add filtered voltage data
    voltageData_.push_back(voltages);

    // Push each label individually
    for (const auto& label : voltageLabels) {
        voltageLabels_.push_back(label); 
    }

    timeData_.push_back(time);

    // Update the min and max for voltages
    for (const auto& voltage : voltages) {
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }
    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//

void GraphPlotting::AddTemperatureDataPoint(const std::vector<float>& temperatures,
    const std::vector<std::string>& tempLabels,
    const wxString& time) {
    temperatureData_.push_back(temperatures);
    for (const auto& label : tempLabels) {
        tempLabels_.push_back(label);
    }
    timeData_.push_back(time);
    for (const auto& temp : temperatures) {
        if (temp > tempMax_) tempMax_ = temp;
        if (temp < tempMin_) tempMin_ = temp;
    }
    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddDiodeCurrentDataPoint(const std::vector<float>& currents, const std::vector<std::string>& labels, const wxString& time) {
    if (currents.size() != labels.size()) {
        wxLogError("Mismatch between the number of diode currents and labels.");
        return;
    }

    timeData_.push_back(time);

    // Ensure diodeCurrentData_ is properly sized
    if (diodeCurrentData_.size() < currents.size()) {
        diodeCurrentData_.resize(currents.size());
        diodeCurrentLabels_ = labels;
    }

    // Update current data and recalculate min/max values
    for (size_t i = 0; i < currents.size(); ++i) {
        diodeCurrentData_[i].push_back(currents[i]);

        // Update the min and max values for diode current
        if (currents[i] > diodeCurrentMax_) diodeCurrentMax_ = currents[i];
        if (currents[i] < diodeCurrentMin_) diodeCurrentMin_ = currents[i];

        if (diodeCurrentData_[i].size() > maxDataPoints_) {
            diodeCurrentData_[i].erase(diodeCurrentData_[i].begin());
        }
    }

    // Maintain time data size
    if (timeData_.size() > maxDataPoints_) {
        timeData_.erase(timeData_.begin());
    }

    RefreshGraph();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddPowerDataPoint(const std::vector<float>& powerReadings, const std::vector<std::string>& labels, const wxString& time) {
    
    powerData_.push_back(powerReadings);

    
    if (powerLabels_.empty() || powerLabels_.size() != labels.size()) {
        powerLabels_ = labels;  
    }
 
    for (const auto& power : powerReadings) {
        if (power > powerMax_) powerMax_ = power;
        if (power < powerMin_) powerMin_ = power;
    }
    timeData_.push_back(time);

    
    if (timeData_.size() > maxDataPoints_) {
        timeData_.erase(timeData_.begin());
    }
    RefreshGraph();
}


//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddSensorDataPoint(const std::vector<float>& sensorReadings, const std::vector<std::string>& labels, const wxString& time) {
    
    sensorData_.push_back(sensorReadings);

   
    if (sensorLabels_.empty() || sensorLabels_.size() != labels.size()) {
        sensorLabels_ = labels;  
    }
    for (const auto& sensor : sensorReadings) {
        if (sensor > sensorMax_) sensorMax_ = sensor;
        if (sensor < sensorMin_) sensorMin_ = sensor;
    }
    timeData_.push_back(time);

    
    if (timeData_.size() > maxDataPoints_) {
        timeData_.erase(timeData_.begin());
        sensorData_.erase(sensorData_.begin());  
    }
    RefreshGraph();
}

void GraphPlotting::render(wxDC& dc) {
    dc.Clear();

    int width, height;
    this->GetSize(&width, &height);
   

    if (currentData_.empty() && voltageData_.empty() && temperatureData_.empty() && diodeCurrentData_.empty() && powerData_.empty() && sensorData_.empty()) {
       
        return;
    }

    const float margin = 0.05;

    float diodeCurrentRange = (diodeCurrentMax_ - diodeCurrentMin_) == 0 ? 1 : (diodeCurrentMax_ - diodeCurrentMin_);
    
    float currentRange = (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_);
    float voltageRange = (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_);
    float tempRange = (tempMax_ - tempMin_) == 0 ? 1 : (tempMax_ - tempMin_);
    float powerRange = (powerMax_ - powerMin_) == 0 ? 1 : (powerMax_ - powerMin_);
    float sensorRange = (sensorMax_ - sensorMin_) == 0 ? 1 : (sensorMax_ - sensorMin_);


    float yScaleDiodeCurrent = (height - 100) / diodeCurrentRange;
  
    float yScaleCurrent = (height - 100) / currentRange;
    float yScaleVoltage = (height - 100) / voltageRange;
    float yScaleTemp = (height - 100) / tempRange;
    float yScalePower = (height - 100) / powerRange;
    float yScaleSensor = (height - 100) / sensorRange;

    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);

    // Define a color palette for different TECs
    std::vector<wxColour> tecColors = { wxColour(255, 0, 0), wxColour(0, 255, 0), wxColour(0, 0, 255), wxColour(255, 255, 0), wxColour(255, 0, 255), wxColour(0, 255, 255) };
    size_t tecColorsSize = tecColors.size();
 
    dc.SetPen(wxPen(*wxBLACK, 2));
    dc.DrawRectangle(50, 50, width - 100, height - 100);  

    dc.SetPen(wxPen(wxColour(200, 200, 200), 1)); 
    for (int i = 0; i <= 10; i++) {
        int yPos = height - 50 - (i * (height - 100) / 10);
        dc.DrawLine(50, yPos, width - 50, yPos);
    }


    // Draw X-axis time labels
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    int visibleLabels = 5;  
    int startIndex = 0;      
    int endIndex = timeData_.size(); 

    
    if (timeData_.size() > visibleLabels) {
        startIndex = static_cast<int>(timeData_.size()) - visibleLabels;
    }

    
    xStep = static_cast<float>(width - 100) / (visibleLabels - 1);

    
    for (size_t i = startIndex; i < endIndex; ++i) {
        int xPos = static_cast<int>((i - startIndex) * xStep + 50);  
        wxString timeLabel = timeData_[i];  
        dc.DrawText(timeLabel, wxPoint(xPos - 10, height - 45));  
    }
    if (alarmTriggered_ && !alarmMessage_.IsEmpty()) {
       
        int xAlarmLine = 100;  
        dc.SetPen(wxPen(wxColour(255, 0, 0), 2)); 
        dc.DrawLine(xAlarmLine, 50, xAlarmLine, height - 50);

        // Draw the alarm message next to the constant red line
        wxString alarmText = wxString::Format("Alarm: %s at %s", alarmMessage_, alarmTime_);
        dc.DrawText(alarmText, wxPoint(xAlarmLine + 5, 55)); 
    }

    // Draw Y-axes
    if (!temperatureData_.empty()) {
        drawYAxisLabels(dc, width, height, true, tempMax_, tempMin_, "°C"); 
    }
    if (!currentData_.empty()) {
        drawYAxisLabels(dc, width, height, true, currentMax_, currentMin_, "A"); 
    }
    if (!voltageData_.empty()) {
        drawYAxisLabels(dc, width, height, false, voltageMax_, voltageMin_, "V");  
    }
    if (!diodeCurrentData_.empty()) {
        drawYAxisLabels(dc, width, height, true, diodeCurrentMax_, diodeCurrentMin_, "A");  
    }
    if (!powerData_.empty()) {
        drawYAxisLabels(dc, width, height, false, powerMax_, powerMin_, "W"); 
    }
    if (!sensorData_.empty()) {
        drawYAxisLabels(dc, width, height, false, sensorMax_, sensorMin_, "L/min");
    }


    if (!diodeCurrentData_.empty()) {
        size_t diodeCount = diodeCurrentData_.size();  // Number of Diodes

        for (size_t diode = 0; diode < diodeCount; ++diode) {
            std::string label = diodeCurrentLabels_[diode];

            if (diodeCount > checkboxes_.size()) return;  // Safety check
            if (!checkboxes_[diode]->IsChecked()) continue;  // Skip unchecked diodes

            wxColour penColor = tecColors[diode % tecColorsSize];  // Cycle through the color palette
            dc.SetPen(wxPen(penColor, 3));  // Set the color for the Diode plot

            float diodeMin = std::numeric_limits<float>::max();
            float diodeMax = std::numeric_limits<float>::min();
            float sumDiodeCurrent = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < diodeCurrentData_[diode].size(); ++i) {
                float diodeCurrentValue = diodeCurrentData_[diode][i];
                diodeMin = std::min(diodeMin, diodeCurrentValue);
                diodeMax = std::max(diodeMax, diodeCurrentValue);
                sumDiodeCurrent += diodeCurrentValue;
                validDataPoints++;

                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i - 1] - diodeCurrentMin_) * yScaleDiodeCurrent);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i] - diodeCurrentMin_) * yScaleDiodeCurrent);
                dc.DrawLine(x1, y1, x2, y2);  // Draw the diode current line
            }

            // Mean and standard deviation
            float diodeMean = validDataPoints > 0 ? sumDiodeCurrent / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < diodeCurrentData_[diode].size(); ++i) {
                float diodeCurrentValue = diodeCurrentData_[diode][i];
                variance += std::pow(diodeCurrentValue - diodeMean, 2);
            }
            float diodeStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            wxString diodeLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Sandard Deviation: %.2f)",
                label, diodeMin, diodeMax, diodeMean, diodeStdDev);
            int xLegendPos = 60 + (diode * 350);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 30, xLegendPos + 30, height - 30);
            dc.DrawText(diodeLabel, wxPoint(xLegendPos + 35, height - 35));
        }
    }




    // Plot the current values
    // Plot the current values with TEC labels and legend
    if (!currentData_.empty()) {
        size_t tecCount = currentData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = currentLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  // Safety check

            if (!checkboxes_[tec]->IsChecked()) continue;  // Skip unchecked TECs

            wxColour penColor = tecColors[tec % tecColorsSize];  // Cycle through the color palette
            dc.SetPen(wxPen(penColor, 3));  // Set the color for the TEC plot

            // Variables to track the min, max, and sum of values for the current TEC
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumCurrent = 0.0;
            size_t validDataPoints = 0;

            // Plot the current graph line and calculate min/max values
            for (size_t i = 1; i < currentData_.size(); ++i) {
                if (tec < currentData_[i - 1].size() && tec < currentData_[i].size()) {
                    float currentValue = currentData_[i][tec];
                    tecMin = std::min(tecMin, currentValue);
                    tecMax = std::max(tecMax, currentValue);
                    sumCurrent += currentValue;  // Add to sum for mean calculation
                    validDataPoints++;  // Count valid data points

                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);
                    dc.DrawLine(x1, y1, x2, y2);  // Draw the current line for the TEC
                }
            }

            // Calculate the mean value
            float tecMean = validDataPoints > 0 ? sumCurrent / validDataPoints : 0.0;

            // Calculate the standard deviation
            float variance = 0.0;
            for (size_t i = 1; i < currentData_.size(); ++i) {
                if (tec < currentData_[i].size()) {
                    float currentValue = currentData_[i][tec];
                    variance += std::pow(currentValue - tecMean, 2);
                }
            }
            float tecStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            
            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                label, tecMin, tecMax, tecMean, tecStdDev);
            int xLegendPos = 50 + (tec * 380); 
            dc.SetPen(wxPen(penColor, 3));  
            dc.DrawLine(xLegendPos, height - 20, xLegendPos + 30, height - 20);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));  
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 40, height - 30));  

        }
    }




    // Plot the voltage values with TEC labels and legend
    if (!voltageData_.empty()) {
        size_t tecCount = voltageData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = voltageLabels_[tec];  // Voltage label for TEC

            if (tec >= checkboxes_.size()) continue;  // Safety check
            if (!checkboxes_[tec]->IsChecked()) continue;  // Skip unchecked TECs

            wxColour penColor = tecColors[tec % tecColorsSize];  // Cycle through the color palette
            dc.SetPen(wxPen(penColor, 3));  // Set the color for the TEC plot

            // Variables for tracking min, max, mean, and standard deviation
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumVoltage = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i - 1].size() && tec < voltageData_[i].size()) {
                    float voltageValue = voltageData_[i][tec];
                    tecMin = std::min(tecMin, voltageValue);
                    tecMax = std::max(tecMax, voltageValue);
                    sumVoltage += voltageValue;
                    validDataPoints++;

                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);
                    dc.DrawLine(x1, y1, x2, y2);  // Draw the voltage line for the TEC
                }
            }

            // Calculate mean and standard deviation
            float tecMean = validDataPoints > 0 ? sumVoltage / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i].size()) {
                    float voltageValue = voltageData_[i][tec];
                    variance += std::pow(voltageValue - tecMean, 2);
                }
            }
            float tecStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            // Draw the legend below the X-axis
            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                label, tecMin, tecMax, tecMean, tecStdDev);
            int xLegendPos = 60 + (tec * 380);  // Adjust legend position horizontally for each TEC
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 30, xLegendPos + 30, height - 30);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 35, height - 35));
        }
    }


    if (!temperatureData_.empty()) {
        size_t tecCount = temperatureData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = tempLabels_[tec];
            if (tec >= checkboxes_.size()) continue;  // Safety check
            if (!checkboxes_[tec]->IsChecked()) continue;  // Skip unchecked TECs

            wxColour penColor = tecColors[tec % tecColorsSize];
            dc.SetPen(wxPen(penColor, 3));  // Set the color for the TEC plot

            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumTemp = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < temperatureData_.size(); ++i) {
                if (tec < temperatureData_[i - 1].size() && tec < temperatureData_[i].size()) {
                    float tempValue = temperatureData_[i][tec];
                    tecMin = std::min(tecMin, tempValue);
                    tecMax = std::max(tecMax, tempValue);
                    sumTemp += tempValue;
                    validDataPoints++;

                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((temperatureData_[i - 1][tec] - tempMin_) * yScaleTemp);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((temperatureData_[i][tec] - tempMin_) * yScaleTemp);
                    dc.DrawLine(x1, y1, x2, y2);  // Draw the temperature line for the TEC
                }
            }

            // Mean and standard deviation
            float tecMean = validDataPoints > 0 ? sumTemp / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < temperatureData_.size(); ++i) {
                if (tec < temperatureData_[i].size()) {
                    float tempValue = temperatureData_[i][tec];
                    variance += std::pow(tempValue - tecMean, 2);
                }
            }
            float tecStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                tempLabels_[tec], tecMin, tecMax, tecMean, tecStdDev);
            int xLegendPos = 60 + (tec * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 30, xLegendPos + 30, height - 30);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 35, height - 35));
        }
    }






    // Plot the power values
    if (!powerData_.empty()) {
        size_t powerMonitorCount = powerData_.front().size();  // Number of Power Monitors

        for (size_t powerMonitor = 0; powerMonitor < powerMonitorCount; ++powerMonitor) {
            std::string label = powerLabels_[powerMonitor];

            if (powerMonitor >= checkboxes_.size()) continue;
            if (!checkboxes_[powerMonitor]->IsChecked()) continue;

            wxColour penColor = tecColors[powerMonitor % tecColorsSize];
            dc.SetPen(wxPen(penColor, 3));

            float powerMin = std::numeric_limits<float>::max();
            float powerMax = std::numeric_limits<float>::min();
            float sumPower = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < powerData_.size(); ++i) {
                float powerValue = powerData_[i][powerMonitor];
                powerMin = std::min(powerMin, powerValue);
                powerMax = std::max(powerMax, powerValue);
                sumPower += powerValue;
                validDataPoints++;

                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((powerData_[i - 1][powerMonitor] - powerMin_) * yScalePower);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((powerData_[i][powerMonitor] - powerMin_) * yScalePower);
                dc.DrawLine(x1, y1, x2, y2);  // Draw the power line
            }

            // Mean and standard deviation
            float powerMean = validDataPoints > 0 ? sumPower / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < powerData_.size(); ++i) {
                float powerValue = powerData_[i][powerMonitor];
                variance += std::pow(powerValue - powerMean, 2);
            }
            float powerStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            wxString powerLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                label, powerMin, powerMax, powerMean, powerStdDev);
            int xLegendPos = 60 + (powerMonitor * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 30, xLegendPos + 30, height - 30);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(powerLabel, wxPoint(xLegendPos + 35, height - 35));
        }
    }




    // Plot the sensor values
    if (!sensorData_.empty()) {
        size_t sensorCount = sensorData_.front().size();  // Number of sensors

        for (size_t sensor = 0; sensor < sensorCount; ++sensor) {
            std::string label = sensorLabels_[sensor];

            if (sensor >= checkboxes_.size()) continue;  // Safety check
            if (!checkboxes_[sensor]->IsChecked()) continue;

            wxColour penColor = tecColors[sensor % tecColorsSize];
            dc.SetPen(wxPen(penColor, 3));

            float sensorMin = std::numeric_limits<float>::max();
            float sensorMax = std::numeric_limits<float>::min();
            float sumSensor = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < sensorData_.size(); ++i) {
                float sensorValue = sensorData_[i][sensor];
                sensorMin = std::min(sensorMin, sensorValue);
                sensorMax = std::max(sensorMax, sensorValue);
                sumSensor += sensorValue;
                validDataPoints++;

                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((sensorData_[i - 1][sensor] - sensorMin_) * yScaleSensor);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((sensorData_[i][sensor] - sensorMin_) * yScaleSensor);
                dc.DrawLine(x1, y1, x2, y2);  // Draw the sensor line
            }

            // Mean and standard deviation
            float sensorMean = validDataPoints > 0 ? sumSensor / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < sensorData_.size(); ++i) {
                float sensorValue = sensorData_[i][sensor];
                variance += std::pow(sensorValue - sensorMean, 2);
            }
            float sensorStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            wxString sensorLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                label, sensorMin, sensorMax, sensorMean, sensorStdDev);
            int xLegendPos = 60 + (sensor * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 30, xLegendPos + 30, height - 30);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(sensorLabel, wxPoint(xLegendPos + 35, height - 35));
        }
    }







}

void GraphPlotting::drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit) {

    int xPos = 25;
    const float maxDisplayValue = 1000;
    float displayMaxValue = std::min(maxDisplayValue, maxValue);
    for (int i = 0; i <= 10; i++) {
        float value = minValue + i * (displayMaxValue - minValue) / 10;
        wxString label = wxString::Format("%.2f %s", value, unit);
        int yPos = height - 50 - (i * (height - 100) / 10);
        dc.DrawText(label, wxPoint(xPos, yPos));
    }
}



void GraphPlotting::paintEvent(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    render(dc);
}
void GraphPlotting::OnResize(wxSizeEvent& event) {
    this->Refresh();
    event.Skip();
}

void GraphPlotting::RefreshGraph() {
    this->Refresh();
    this->Update();
}

