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
    voltageMin_(std::numeric_limits<float>::max()) {  // Properly initializing vector of checkboxes

    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
   
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents,
    const std::vector<std::string>& currentLabels, const wxString& time) {

    currentData_.push_back(currents);

    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label);  // Push current labels individually
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
        voltageLabels_.push_back(label);  // Push voltage labels individually
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

void GraphPlotting::render(wxDC& dc) {
    dc.Clear();

    int width, height;
    this->GetSize(&width, &height);

    // Log size of the drawing area
    //wxLogMessage("Rendering graph: width = %d, height = %d", width, height);

    if (currentData_.empty() && voltageData_.empty() && temperatureData_.empty() && diodeCurrentData_.empty()) {
        //wxLogMessage("No data to plot.");
        return;
    }

    const float margin = 0.05;

    // Set ranges for current, voltage, and temperature
    //wxLogMessage("diodeCurrentMax: %f, diodeCurrentMin: %f, diodeCurrentRange: %f", diodeCurrentMax_, diodeCurrentMin_, diodeCurrentMax_ - diodeCurrentMin_);

    float diodeCurrentRange = (diodeCurrentMax_ - diodeCurrentMin_) == 0 ? 1 : (diodeCurrentMax_ - diodeCurrentMin_);
    //wxLogMessage("Diode Current Range: %f", diodeCurrentRange);
    float currentRange = (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_);
    float voltageRange = (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_);
    float tempRange = (tempMax_ - tempMin_) == 0 ? 1 : (tempMax_ - tempMin_);


    float yScaleDiodeCurrent = (height - 100) / diodeCurrentRange;
   // wxLogMessage("Y Scale for Diode Current: %f", yScaleDiodeCurrent);
    float yScaleCurrent = (height - 100) / currentRange;
    float yScaleVoltage = (height - 100) / voltageRange;
    float yScaleTemp = (height - 100) / tempRange;

    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);
    //wxLogMessage("X Step for Diode Plotting: %f", xStep);
    // Draw the graph box
    dc.SetPen(wxPen(*wxBLACK, 2));
    dc.DrawRectangle(50, 50, width - 100, height - 100);  // Draw graph boundary box

    // Major Y-axis grid lines (light gray)
    dc.SetPen(wxPen(wxColour(200, 200, 200), 1));  // Light gray for grid lines
    for (int i = 0; i <= 10; i++) {
        int yPos = height - 50 - (i * (height - 100) / 10);
        dc.DrawLine(50, yPos, width - 50, yPos);
    }
   

    // Draw X-axis time labels
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    int visibleLabels = 5;  // Limit the number of visible time labels (you can adjust this)
    int startIndex = 0;      // Start from this index in timeData_
    int endIndex = timeData_.size();  // End at the last index

    // If there are more time points than visible labels, shift the start index
    if (timeData_.size() > visibleLabels) {
        startIndex = static_cast<int>(timeData_.size()) - visibleLabels;
    }

    // Calculate new xStep based on visible time points for proper spacing
    xStep = static_cast<float>(width - 100) / (visibleLabels - 1);

    // Draw time labels at regular intervals along the X-axis
    for (size_t i = startIndex; i < endIndex; ++i) {
        int xPos = static_cast<int>((i - startIndex) * xStep + 50);  // Position of the label on the X-axis
        wxString timeLabel = timeData_[i];  // Get the time label from timeData_
        dc.DrawText(timeLabel, wxPoint(xPos - 10, height - 45));  // Draw the time label slightly below the X-axis
    }


    // Draw Y-axes
    if (!temperatureData_.empty()) {
        drawYAxisLabels(dc, width, height, true, tempMax_, tempMin_, "°C");  // Left Y-axis for temperature
    }
    if (!currentData_.empty()) {
        drawYAxisLabels(dc, width, height, true, currentMax_, currentMin_, "A");  // Left Y-axis for current
    }
    if (!voltageData_.empty()) {
        drawYAxisLabels(dc, width, height, false, voltageMax_, voltageMin_, "V");  // Right Y-axis for voltage
    }
    if (!diodeCurrentData_.empty()) {
        drawYAxisLabels(dc, width, height, true, diodeCurrentMax_, diodeCurrentMin_, "A");  // Left Y-axis for Diode Current in Amperes
    }


    if (!diodeCurrentData_.empty()) {
        size_t diodeCount = diodeCurrentData_.size();  // Number of Diodes

       /* // Safety check for checkbox size
        if (diodeCount > checkboxes_.size()) {
            wxLogMessage("Mismatch between diode count (%d) and checkbox size (%d). Aborting diode plotting.", diodeCount, checkboxes_.size());
            return;
        }*/

        for (size_t diode = 0; diode < diodeCount; ++diode) {
            std::string label = diodeCurrentLabels_[diode];

           
            if (diodeCount > checkboxes_.size()) return;
            // Skip unchecked diodes
            if (!checkboxes_[diode]->IsChecked()) continue;
            

           // wxLogMessage("Plotting data for diode %d with label: %s", diode, label);
            dc.SetPen(wxPen(wxColour(0, 0, 255), 2));  // Blue for diode current

            for (size_t i = 1; i < diodeCurrentData_[diode].size(); ++i) {
                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i - 1] - diodeCurrentMin_) * yScaleDiodeCurrent);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((diodeCurrentData_[diode][i] - diodeCurrentMin_) * yScaleDiodeCurrent);

               // wxLogMessage("Plotting line from (%d, %d) to (%d, %d) for diode %d", x1, y1, x2, y2, diode);
                dc.DrawLine(x1, y1, x2, y2);
            }
        }
    }
    else {
        //wxLogMessage("Diode current data is empty. No diode data to plot.");
    }



    // Plot the current values
    if (!currentData_.empty()) {
        size_t tecCount = currentData_.front().size();  // Number of TECs
        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = currentLabels_[tec];
            if (tecCount > checkboxes_.size()) return;  // Safety check

            if (!checkboxes_[tec]->IsChecked()) continue;  // Skip unchecked TECs

            dc.SetPen(wxPen(wxColour(255, 0, 0), 2));  // Red line for current
            for (size_t i = 1; i < currentData_.size(); ++i) {
                if (tec < currentData_[i - 1].size() && tec < currentData_[i].size()) {
                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);
                    dc.DrawLine(x1, y1, x2, y2);
                }
            }
        }
    }

    // Plot the voltage values
   // Plot the voltage values
    if (!voltageData_.empty()) {
        size_t tecCount = voltageData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            std::string label = voltageLabels_[tec];  // Voltage label for TEC

            // Ensure that the tec index is within the bounds of checkboxes_
            if (tec >= checkboxes_.size()) {
                //wxLogError("TEC index out of bounds: %d for checkboxes size: %d", tec, checkboxes_.size());
                continue;  // Skip this TEC if index is out of bounds
            }

            // Skip unchecked TECs
            if (!checkboxes_[tec]->IsChecked()) continue;

            dc.SetPen(wxPen(wxColour(0, 255, 0), 2));  // Green line for voltage

            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i - 1].size() && tec < voltageData_[i].size()) {
                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);
                    dc.DrawLine(x1, y1, x2, y2);
                }
            }
        }
    }


    // Plot the temperature values
    if (!temperatureData_.empty()) {
        size_t tecCount = temperatureData_.front().size();  // Number of TECs
        for (size_t tec = 0; tec < tecCount; ++tec) {
            if (tec >= checkboxes_.size()) {
                //wxLogError("TEC index out of bounds: %zu for checkboxes size: %zu", tec, checkboxes_.size());
                continue;  // Skip this iteration to prevent crash
            }
            if (!checkboxes_[tec]->IsChecked()) continue;  // Skip unchecked TECs
            dc.SetPen(wxPen(wxColour(255, 128, 0), 2));  // Orange line for temperature
            for (size_t i = 1; i < temperatureData_.size(); ++i) {
                if (tec < temperatureData_[i - 1].size() && tec < temperatureData_[i].size()) {
                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((temperatureData_[i - 1][tec] - tempMin_) * yScaleTemp);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((temperatureData_[i][tec] - tempMin_) * yScaleTemp);
                    dc.DrawLine(x1, y1, x2, y2);
                }
            }
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

