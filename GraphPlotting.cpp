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
void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents,const std::vector<std::string>& currentLabels, const wxString& time) {

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
void GraphPlotting::AddVoltageDataPoint(const std::vector<float>& voltages,const std::vector<std::string>& voltageLabels,const wxString& time) {
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

void GraphPlotting::AddTemperatureDataPoint(const std::vector<float>& temperatures,const std::vector<std::string>& tempLabels,const wxString& time) {
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

    
    if (diodeCurrentData_.size() < currents.size()) {
        diodeCurrentData_.resize(currents.size());
        diodeCurrentLabels_ = labels;
    }

    
    for (size_t i = 0; i < currents.size(); ++i) {
        diodeCurrentData_[i].push_back(currents[i]);

        
        if (currents[i] > diodeCurrentMax_) diodeCurrentMax_ = currents[i];
        if (currents[i] < diodeCurrentMin_) diodeCurrentMin_ = currents[i];

        if (diodeCurrentData_[i].size() > maxDataPoints_) {
            diodeCurrentData_[i].erase(diodeCurrentData_[i].begin());
        }
    }

    
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

    
    wxDateTime startTime, endTime;
    if (!startTime.ParseFormat(timeData_.front(), "%H:%M:%S")) {
        wxLogError("Failed to parse start time: %s", timeData_.front());
    }
    if (!endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
        wxLogError("Failed to parse end time: %s", timeData_.back());
    }

    wxTimeSpan totalDuration = endTime.Subtract(startTime);
    long totalDurationInSeconds = totalDuration.GetSeconds().ToLong();

    
    if (totalDurationInSeconds <= 0) {
        return;
    }

    float xStep = static_cast<float>(width - 100) / totalDurationInSeconds;//calculates the xStep for Time Axis...xStep determines the horizontal distance (in pixels) between two consecutive data points, scaled to fit the panel width (width - 100 for margins).
    const float margin = 0.05;

    float diodeCurrentRange = (diodeCurrentMax_ - diodeCurrentMin_) == 0 ? 1 : (diodeCurrentMax_ - diodeCurrentMin_);//Calculate Y-Axis Ranges
    float currentRange = (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_);
    float voltageRange = (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_);
    float tempRange = (tempMax_ - tempMin_) == 0 ? 1 : (tempMax_ - tempMin_);
    float powerRange = (powerMax_ - powerMin_) == 0 ? 1 : (powerMax_ - powerMin_);
    float sensorRange = (sensorMax_ - sensorMin_) == 0 ? 1 : (sensorMax_ - sensorMin_);


    float yScaleDiodeCurrent = (height - 100) / diodeCurrentRange;// Calculate Y-Scale for Plotting....Converts data values into pixel positions based on the Y-axis range.Scales the data to fit within the panel height(height - 100 for margins).
    float yScaleCurrent = (height - 100) / currentRange;
    float yScaleVoltage = (height - 100) / voltageRange;
    float yScaleTemp = (height - 100) / tempRange;
    float yScalePower = (height - 100) / powerRange;
    float yScaleSensor = (height - 100) / sensorRange;

    std::vector<wxColour> tecColors = { wxColour(0, 0, 255),wxColour(255, 165, 0), wxColour(0, 0, 255), wxColour(255, 255, 0), wxColour(255, 0, 255), wxColour(0, 255, 255) };
    size_t tecColorsSize = tecColors.size();//Define Colors for TEC Data....

    dc.SetPen(wxPen(*wxBLACK, 2));//Draw the Plot Area....Draws a rectangular boundary for the graph area with a 2-pixel black border.
    dc.DrawRectangle(50, 50, width - 100, height - 100);

    dc.SetPen(wxPen(wxColour(200, 200, 200), 1));//Draw Gridlines
    for (int i = 0; i <= 10; i++) {//Draws horizontal gridlines within the plot area. Spacing is based on dividing the plot area height(height - 100) into 10 equal parts.
        int yPos = height - 50 - (i * (height - 100) / 10);
        dc.DrawLine(50, yPos, width - 50, yPos);
    }

    // Draw time labels based on on X-Axis
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    wxDateTime currentTime = startTime; 
    int visibleLabels = 10; 
    long labelInterval = totalDurationInSeconds / visibleLabels;  

    for (int i = 0; i <= visibleLabels; ++i) {
        int xPos = static_cast<int>(i * labelInterval * xStep + 50);  
        wxString timeLabel = currentTime.Format("%H:%M:%S");  
        dc.DrawText(timeLabel, wxPoint(xPos - 10, height - 45)); 
        currentTime.Add(wxTimeSpan::Seconds(labelInterval));  
    }




    // Alarm handling
    if (!alarmMessages_.empty() && !alarmTimes_.empty()) {//we ensures that there are alarms to process, if it is empty  we skip the code
        for (size_t i = 0; i < alarmMessages_.size(); ++i) {
            wxDateTime alarmDateTime;
            if (!alarmDateTime.ParseFormat(alarmTimes_[i], "%H:%M:%S")) {
                wxLogError("Failed to parse alarm time: %s", alarmTimes_[i]);
                continue;
            }

            // Calculate the x position for the alarm line
            wxTimeSpan timeSinceStart = alarmDateTime.Subtract(startTime);
            long secondsSinceStart = timeSinceStart.GetSeconds().ToLong();
            int alarmXPosition = static_cast<int>(secondsSinceStart * xStep + 50);

            // Check if alarmXPosition is within the graph bounds
            if (alarmXPosition >= 50 && alarmXPosition <= width - 50) {
                // Set up the darker red dashed pen for the alarm line
                wxPen darkRedPen(wxColour(139, 0, 0), 2, wxPENSTYLE_SHORT_DASH);
                dc.SetPen(darkRedPen);

                // Draw the red dashed line at the alarm time
                dc.DrawLine(alarmXPosition, 50, alarmXPosition, height - 50);

                // Draw a gradient effect to highlight the error
                wxColour startColour(255, 69, 0);  
                wxColour endColour(139, 0, 0);    
                wxBrush gradientBrush(wxColour(255, 0, 0, 128));  
                dc.SetBrush(gradientBrush);
                dc.GradientFillLinear(wxRect(alarmXPosition - 5, 50, 10, height - 100), startColour, endColour, wxSOUTH);

                // Draw the alarm message next to the line
                wxString alarmText = wxString::Format("Alarm: %s at %s", alarmMessages_[i], alarmTimes_[i]);
                dc.SetTextForeground(wxColour(139, 0, 0));  
                dc.DrawText(alarmText, wxPoint(alarmXPosition + 5, 55));
            }
        }
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


    // Plot the current values
   // Plot the current values with TEC labels and legend
    if (!currentData_.empty()) {
        size_t tecCount = currentData_.front().size();
        //wxLogMessage("Number of TEC data sets received: %zu", tecCount);

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = currentLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  

            if (!checkboxes_[tec]->IsChecked()) continue; 

            wxColour penColor = tecColors[tec % tecColorsSize];  
            dc.SetPen(wxPen(penColor, 3));  

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
                    sumCurrent += currentValue;  
                    validDataPoints++;  

                    // Use actual timestamps from timeData_ to calculate x1 and x2
                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest value at the right end of the TEC plot
            if (!currentData_.empty() && tec < currentData_.back().size()) {
                float latestValue = currentData_.back()[tec];

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestValue - currentMin_) * yScaleCurrent);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
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
    // Plot the voltage values
    if (!voltageData_.empty()) {
        size_t tecCount = voltageData_.front().size();  

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = voltageLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  

            if (!checkboxes_[tec]->IsChecked()) continue;  

            wxColour penColor = tecColors[tec % tecColorsSize];  
            dc.SetPen(wxPen(penColor, 3)); 

            // Variables to track the min, max, and sum of values for the current TEC
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumVoltage = 0.0;
            size_t validDataPoints = 0;

            // Plot the voltage graph line and calculate min/max values
            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i - 1].size() && tec < voltageData_[i].size()) {
                    float voltageValue = voltageData_[i][tec];
                    tecMin = std::min(tecMin, voltageValue);
                    tecMax = std::max(tecMax, voltageValue);
                    sumVoltage += voltageValue;  
                    validDataPoints++;  

                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest voltage value at the right end of the line
            if (!voltageData_.empty() && tec < voltageData_.back().size()) {
                float latestVoltageValue = voltageData_.back()[tec];

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestVoltageValue - voltageMin_) * yScaleVoltage);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestVoltageValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
                }
            }

            // Draw the legend below the X-axis (color and TEC label along with min/max values)
            float tecMean = validDataPoints > 0 ? sumVoltage / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i].size()) {
                    float voltageValue = voltageData_[i][tec];
                    variance += std::pow(voltageValue - tecMean, 2);
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
    // Plot the temperature values
    if (!temperatureData_.empty()) {
        size_t tecCount = temperatureData_.front().size();  

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = tempLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  

            if (!checkboxes_[tec]->IsChecked()) continue; 

            wxColour penColor = tecColors[tec % tecColorsSize];  
            dc.SetPen(wxPen(penColor, 3));  

            // Variables to track the min, max, and sum of values for the current TEC
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumTemp = 0.0;
            size_t validDataPoints = 0;

            // Plot the temperature graph line and calculate min/max values
            for (size_t i = 1; i < temperatureData_.size(); ++i) {
                if (tec < temperatureData_[i - 1].size() && tec < temperatureData_[i].size()) {
                    float tempValue = temperatureData_[i][tec];
                    tecMin = std::min(tecMin, tempValue);
                    tecMax = std::max(tecMax, tempValue);
                    sumTemp += tempValue;  
                    validDataPoints++;  

                    // Use actual timestamps from timeData_ to calculate x1 and x2
                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((temperatureData_[i - 1][tec] - tempMin_) * yScaleTemp);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((temperatureData_[i][tec] - tempMin_) * yScaleTemp);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest temperature value at the right end of the line
            if (!temperatureData_.empty() && tec < temperatureData_.back().size()) {
                float latestTempValue = temperatureData_.back()[tec];

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestTempValue - tempMin_) * yScaleTemp);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestTempValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
                }
            }

            // Draw the legend below the X-axis (color and TEC label along with min/max values)
            float tecMean = validDataPoints > 0 ? sumTemp / validDataPoints : 0.0;
            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f)",
                label, tecMin, tecMax, tecMean);

            int xLegendPos = 50 + (tec * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 20, xLegendPos + 30, height - 20);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 40, height - 30));
        }
    }
    if (!diodeCurrentData_.empty()) {
        size_t diodeCount = diodeCurrentData_.size();  

        for (size_t diode = 0; diode < diodeCount; ++diode) {
            std::string label = diodeCurrentLabels_[diode];

            if (diodeCount > checkboxes_.size()) return;  
            if (!checkboxes_[diode]->IsChecked()) continue;  

            wxColour penColor = tecColors[diode % tecColorsSize];  
            dc.SetPen(wxPen(penColor, 3));  
            float diodeMin = std::numeric_limits<float>::max();
            float diodeMax = std::numeric_limits<float>::min();
            float sumDiodeCurrent = 0.0;
            size_t validDataPoints = 0;

            for (size_t i = 1; i < diodeCurrentData_[diode].size(); ++i) {
                if (i < diodeCurrentData_[diode].size()) {
                    float diodeCurrentValue = diodeCurrentData_[diode][i];
                    diodeMin = std::min(diodeMin, diodeCurrentValue);
                    diodeMax = std::max(diodeMax, diodeCurrentValue);
                    sumDiodeCurrent += diodeCurrentValue;
                    validDataPoints++;

                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i - 1] - diodeCurrentMin_) * yScaleDiodeCurrent);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i] - diodeCurrentMin_) * yScaleDiodeCurrent);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest diode current value at the right end of the line
            if (!diodeCurrentData_.empty() && diode < diodeCurrentData_[diode].size()) {
                float latestDiodeCurrentValue = diodeCurrentData_[diode].back();

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestDiodeCurrentValue - diodeCurrentMin_) * yScaleDiodeCurrent);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestDiodeCurrentValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
                }
            }

            // Mean and standard deviation
            float diodeMean = validDataPoints > 0 ? sumDiodeCurrent / validDataPoints : 0.0;
            float variance = 0.0;
            for (size_t i = 1; i < diodeCurrentData_[diode].size(); ++i) {
                float diodeCurrentValue = diodeCurrentData_[diode][i];
                variance += std::pow(diodeCurrentValue - diodeMean, 2);
            }
            float diodeStdDev = validDataPoints > 1 ? std::sqrt(variance / (validDataPoints - 1)) : 0.0;

            wxString diodeLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f, Standard Deviation: %.2f)",
                label, diodeMin, diodeMax, diodeMean, diodeStdDev);
            int xLegendPos = 50 + (diode * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 20, xLegendPos + 30, height - 20);
            dc.DrawText(diodeLabel, wxPoint(xLegendPos + 40, height - 30));

            
            
        }
    }
    // Plot the power values
    if (!powerData_.empty()) {
        size_t tecCount = powerData_.front().size();  

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = powerLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  

            if (!checkboxes_[tec]->IsChecked()) continue;  

            wxColour penColor = tecColors[tec % tecColorsSize]; 
            dc.SetPen(wxPen(penColor, 3));

            // Variables to track the min, max, and sum of values for the current TEC
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumPower = 0.0;
            size_t validDataPoints = 0;

            // Plot the power graph line and calculate min/max values
            for (size_t i = 1; i < powerData_.size(); ++i) {
                if (tec < powerData_[i - 1].size() && tec < powerData_[i].size()) {
                    float powerValue = powerData_[i][tec];
                    tecMin = std::min(tecMin, powerValue);
                    tecMax = std::max(tecMax, powerValue);
                    sumPower += powerValue;  
                    validDataPoints++; 

                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((powerData_[i - 1][tec] - powerMin_) * yScalePower);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((powerData_[i][tec] - powerMin_) * yScalePower);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest power value at the right end of the line
            if (!powerData_.empty() && tec < powerData_.back().size()) {
                float latestPowerValue = powerData_.back()[tec];

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestPowerValue - powerMin_) * yScalePower);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestPowerValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
                }
            }

            // Draw the legend below the X-axis (color and TEC label along with min/max values)
            float tecMean = validDataPoints > 0 ? sumPower / validDataPoints : 0.0;
            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f)", label, tecMin, tecMax, tecMean);

            int xLegendPos = 50 + (tec * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 20, xLegendPos + 30, height - 20);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 40, height - 30));
        }
    }
    // Plot the sensor values
    if (!sensorData_.empty()) {
        size_t tecCount = sensorData_.front().size(); 

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = sensorLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  

            if (!checkboxes_[tec]->IsChecked()) continue;

            wxColour penColor = tecColors[tec % tecColorsSize];  
            dc.SetPen(wxPen(penColor, 3));  

            // Variables to track the min, max, and sum of values for the current TEC
            float tecMin = std::numeric_limits<float>::max();
            float tecMax = std::numeric_limits<float>::min();
            float sumSensor = 0.0;
            size_t validDataPoints = 0;

            // Plot the sensor graph line and calculate min/max values
            for (size_t i = 1; i < sensorData_.size(); ++i) {
                if (tec < sensorData_[i - 1].size() && tec < sensorData_[i].size()) {
                    float sensorValue = sensorData_[i][tec];
                    tecMin = std::min(tecMin, sensorValue);
                    tecMax = std::max(tecMax, sensorValue);
                    sumSensor += sensorValue;  // Add to sum for mean calculation
                    validDataPoints++;  // Count valid data points

                    wxDateTime time1, time2;
                    if (time1.ParseFormat(timeData_[i - 1], "%H:%M:%S") && time2.ParseFormat(timeData_[i], "%H:%M:%S")) {
                        wxTimeSpan diff1 = time1.Subtract(startTime);
                        wxTimeSpan diff2 = time2.Subtract(startTime);

                        long seconds1 = diff1.GetSeconds().ToLong();
                        long seconds2 = diff2.GetSeconds().ToLong();

                        int x1 = static_cast<int>((seconds1 * xStep) + 50);
                        int y1 = height - 50 - static_cast<int>((sensorData_[i - 1][tec] - sensorMin_) * yScaleSensor);

                        int x2 = static_cast<int>((seconds2 * xStep) + 50);
                        int y2 = height - 50 - static_cast<int>((sensorData_[i][tec] - sensorMin_) * yScaleSensor);

                        // Draw the current line for the TEC
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }

            // Display the latest sensor value at the right end of the line
            if (!sensorData_.empty() && tec < sensorData_.back().size()) {
                float latestSensorValue = sensorData_.back()[tec];

                wxDateTime endTime;
                if (endTime.ParseFormat(timeData_.back(), "%H:%M:%S")) {
                    wxTimeSpan endDiff = endTime.Subtract(startTime);
                    long endSeconds = endDiff.GetSeconds().ToLong();
                    int xEnd = static_cast<int>((endSeconds * xStep) + 50);
                    int yEnd = height - 50 - static_cast<int>((latestSensorValue - sensorMin_) * yScaleSensor);

                    // Draw the current value at the end of the line
                    wxString currentValueLabel = wxString::Format("%.2f", latestSensorValue);
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                    dc.DrawText(currentValueLabel, wxPoint(xEnd + 5, yEnd - 10));
                }
            }

            // Draw the legend below the X-axis (color and TEC label along with min/max values)
            float tecMean = validDataPoints > 0 ? sumSensor / validDataPoints : 0.0;
            wxString tecLabel = wxString::Format("%s (Min: %.2f, Max: %.2f, Mean: %.2f)", label, tecMin, tecMax, tecMean);

            int xLegendPos = 50 + (tec * 380);
            dc.SetPen(wxPen(penColor, 3));
            dc.DrawLine(xLegendPos, height - 20, xLegendPos + 30, height - 20);
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(tecLabel, wxPoint(xLegendPos + 40, height - 30));
        }
    }
}

void GraphPlotting::drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit) {// Determine Label Positions,,Divides the Y-axis into 10 intervals and places labels at each interval.

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

