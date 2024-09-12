#include "GraphPlotting.h"
#include "wx/dcbuffer.h"
#include <algorithm>
#include <limits>

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
EVT_PAINT(GraphPlotting::paintEvent)
EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, winid, pos, size),
    currentMax_(std::numeric_limits<float>::lowest()),
    currentMin_(std::numeric_limits<float>::max()),
    voltageMax_(std::numeric_limits<float>::lowest()),
    voltageMin_(std::numeric_limits<float>::max()) {
    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void GraphPlotting::AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time) {
    const float maxReasonableValue = 1000;  // Adjust this threshold based on realistic max value
    const float minReasonableValue = -1000;

    auto isValid = [minReasonableValue, maxReasonableValue](float value) {
        return value > minReasonableValue && value < maxReasonableValue;
        };



    std::vector<float> filteredCurrents, filteredVoltages;
    
    // Filter currents
    std::copy_if(currents.begin(), currents.end(), std::back_inserter(filteredCurrents), isValid);

    // Filter voltages
    std::copy_if(voltages.begin(), voltages.end(), std::back_inserter(filteredVoltages), isValid);

    // Add filtered data
    currentData_.push_back(filteredCurrents);
    voltageData_.push_back(filteredVoltages);
    timeData_.push_back(time);

    // Update the min and max for currents and voltages with logging for large values
    for (auto& current : currents) {
        if (current > maxReasonableValue) {
            wxLogError("Unusually high current detected: %f", current);
        }
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    for (auto& voltage : voltages) {
        if (voltage > maxReasonableValue || voltage < minReasonableValue) {
            wxLogError("Unusually high or low voltage detected: %f", voltage);
        }
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }

    const int maxVisiblePoints = 50;
    if (timeData_.size() > maxVisiblePoints) {
        timeData_.erase(timeData_.begin());
        currentData_.erase(currentData_.begin());
        voltageData_.erase(voltageData_.begin());
    }

    RefreshGraph();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


     //Add Method to Handle Only Currents Category:
void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents, const wxString& time) {
    const float maxReasonableValue = 1000;  // Adjust this threshold based on realistic max value

    auto isValid = [](float value) { return value > -1000 && value < 1000; };

    std::vector<float> filteredCurrents;
    std::copy_if(currents.begin(), currents.end(), std::back_inserter(filteredCurrents), isValid);

    // Add filtered current data
    currentData_.push_back(filteredCurrents);
    timeData_.push_back(time);

    // Update min/max for currents
    for (auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    const int maxVisiblePoints = 50;
    if (timeData_.size() > maxVisiblePoints) {
        timeData_.erase(timeData_.begin());
        currentData_.erase(currentData_.begin());
    }

    RefreshGraph();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Add Method to Handle Only Voltages
void GraphPlotting::AddVoltageDataPoint(const std::vector<float>& voltages, const wxString& time) {
    const float maxReasonableValue = 1000;  // Adjust this threshold based on realistic max value

    auto isValid = [](float value) { return value > -1000 && value < 1000; };

    std::vector<float> filteredVoltages;
    std::copy_if(voltages.begin(), voltages.end(), std::back_inserter(filteredVoltages), isValid);

    // Add filtered voltage data
    voltageData_.push_back(filteredVoltages);
    timeData_.push_back(time);

    // Update min/max for voltages
    for (auto& voltage : voltages) {
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }

    const int maxVisiblePoints = 50;
    if (timeData_.size() > maxVisiblePoints) {
        timeData_.erase(timeData_.begin());
        voltageData_.erase(voltageData_.begin());
    }

    RefreshGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void GraphPlotting::render(wxDC& dc) {
    dc.Clear();  // Clear the canvas before drawing

    int width, height;
    this->GetSize(&width, &height);  // Get the dimensions of the window

    if (currentData_.empty() && voltageData_.empty()) return;

    // Dynamic range based on actual data
    const float maxRangeCap = 100;  // Cap the maximum range to avoid excessive scaling
    float currentRange = std::min(maxRangeCap, (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_));
    float voltageRange = std::min(maxRangeCap, (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_));

    float yScaleCurrent = (height - 50) / currentRange;
    float yScaleVoltage = (height - 50) / voltageRange;
    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);

    // Draw grids and labels
    drawGridLines(dc, width, height);
    drawYAxisLabels(dc, width, height, true, currentMax_, currentMin_, "A");  // Use dynamic current range
    drawYAxisLabels(dc, width, height, false, voltageMax_, voltageMin_, "V"); // Use dynamic voltage range

    // Your existing drawing logic for currents and voltages



    // Define colors for TECs
    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),    // Red for TEC 1 Current
        wxColour(0, 255, 0),    // Green for TEC 1 Voltage
        wxColour(0, 0, 255),    // Blue for TEC 2 Current
        wxColour(255, 255, 0),  // Yellow for TEC 2 Voltage
        wxColour(0, 255, 255),  // Cyan for TEC 3 Current (if needed)
        wxColour(255, 0, 255)   // Magenta for TEC 3 Voltage (if needed)
    };

    // Plot current and voltage values for each TEC

// Check if currentData_ is empty before accessing it
    if (!currentData_.empty()) {
        for (size_t tec = 0; tec < currentData_.front().size(); ++tec) {
            // Plot current values
            dc.SetPen(wxPen(colors[tec * 2 % colors.size()], 2));  // Correct indexing for current
            for (size_t i = 1; i < currentData_.size(); ++i) {
                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);
                dc.DrawLine(x1, y1, x2, y2);
            }
        }
    }

    // Plot voltage values
    if (!voltageData_.empty()) {  // Ensure voltageData_ is not empty before plotting
        for (size_t tec = 0; tec < voltageData_.front().size(); ++tec) {
            dc.SetPen(wxPen(colors[tec * 2 + 1 % colors.size()], 2));  // Correct indexing for voltage
            for (size_t i = 1; i < voltageData_.size(); ++i) {
                int x1 = static_cast<int>((i - 1) * xStep + 50);
                int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);
                int x2 = static_cast<int>(i * xStep + 50);
                int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);
                dc.DrawLine(x1, y1, x2, y2);
            }
        }
    }


    // Draw axes labels and legend
    drawAxesLabels(dc, width, height);
    drawLegend(dc, width);
}






void GraphPlotting::drawGridLines(wxDC& dc, int width, int height) {
    dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));  // Light gray dotted lines for grid

    // Horizontal grid lines (Y-axis)
    for (int i = 1; i <= 10; ++i) {
        int y = height - 50 - static_cast<int>((i * height) / 10);
        dc.DrawLine(50, y, width, y);  // Horizontal lines
    }

    // Vertical grid lines (X-axis) with selective time labels to avoid overlap
    if (!timeData_.empty() && timeData_.size() > 1) {
        float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);  // X-axis scaling
        size_t step = std::max(1, static_cast<int>(timeData_.size() / (width / 150)));  // Adjust label spacing based on width

        for (size_t i = 0; i < timeData_.size(); i += step) {
            int x = static_cast<int>(i * xStep + 50);  // Calculate X position
            dc.DrawLine(x, 0, x, height - 50);  // Draw vertical lines

            // Draw time labels selectively (based on calculated step)
            wxString timeLabel = timeData_[i];  // Get the time from the deque
            dc.DrawText(timeLabel, wxPoint(x - 10, height - 40));  // Adjust X and Y for the label
        }
    }
}




void GraphPlotting::drawLegend(wxDC& dc, int width) {
    int legendYPosition = 15;

    // Ensure unique colors are used for current and voltage lines
    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),    // Red for TEC 1 Current
        wxColour(0, 255, 0),    // Green for TEC 1 Voltage
        wxColour(0, 0, 255),    // Blue for TEC 2 Current
        wxColour(255, 255, 0),  // Yellow for TEC 2 Voltage
        wxColour(0, 255, 255),  // Cyan for TEC 3 Current (if needed)
        wxColour(255, 0, 255)   // Magenta for TEC 3 Voltage (if needed)
    };

    // Plot legend for current values for each TEC
    if (!currentData_.empty()) {  // Check if current data exists
        for (size_t tec = 0; tec < currentData_.front().size(); ++tec) {
            // Current
            dc.SetPen(wxPen(colors[tec * 2 % colors.size()], 2));  // Multiply index by 2 for current
            dc.DrawLine(width - 100, legendYPosition, width - 80, legendYPosition);
            dc.DrawText(wxString::Format("TEC %d Current", tec + 1), wxPoint(width - 75, legendYPosition - 5));
            legendYPosition += 20;
        }
    }

    // Plot legend for voltage values for each TEC
    if (!voltageData_.empty()) {  // Check if voltage data exists
        for (size_t tec = 0; tec < voltageData_.front().size(); ++tec) {
            // Voltage
            dc.SetPen(wxPen(colors[tec * 2 + 1 % colors.size()], 2));  // Correct indexing for voltage
            dc.DrawLine(width - 100, legendYPosition, width - 80, legendYPosition);
            dc.DrawText(wxString::Format("TEC %d Voltage", tec + 1), wxPoint(width - 75, legendYPosition - 5));
            legendYPosition += 20;
        }
    }
}


void GraphPlotting::drawAxesLabels(wxDC& dc, int width, int height) {
    wxFont font(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    dc.DrawText("Current/Voltage (A/V)", wxPoint(200, height - 150));  // Y-axis label
    dc.DrawText("Time (hh:mm:ss)", wxPoint(width / 2 - 50, height + 20));  // X-axis label
}

void GraphPlotting::drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit) {
    int xPos = leftAxis ? 50 : width - 50;  // Position on the left or right of the panel
    const float maxDisplayValue = 1000;  // Set a cap for max value displayed on Y-axis
    float displayMaxValue = std::min(maxDisplayValue, maxValue);

    for (int i = 0; i <= 10; i++) {
        float value = minValue + i * (displayMaxValue - minValue) / 10;

        wxString label = wxString::Format("%.2f %s", value, unit);
        int yPos = height - 50 - (i * (height - 100) / 10);
        dc.DrawText(label, xPos, yPos);
    }
}


void GraphPlotting::paintEvent(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);  // Use buffered DC to avoid flickering
    render(dc);  // Call the render method to handle the actual drawing
}

void GraphPlotting::OnResize(wxSizeEvent& event) {
    this->Refresh();
    event.Skip();
}

void GraphPlotting::RefreshGraph() {
    this->Refresh();  // Trigger a repaint
    this->Update();   // Ensure the repaint happens immediately
}