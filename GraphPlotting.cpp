#include "GraphPlotting.h"
#include "wx/dcbuffer.h"
#include <algorithm>
#include <limits>

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
EVT_PAINT(GraphPlotting::paintEvent)
EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, const std::map<std::string, wxCheckBox*>& tecCheckboxes)
    : wxPanel(parent, winid, pos, size), tecCheckboxes_(tecCheckboxes),
    currentMax_(std::numeric_limits<float>::lowest()),
    currentMin_(std::numeric_limits<float>::max()),
    voltageMax_(std::numeric_limits<float>::lowest()),
    voltageMin_(std::numeric_limits<float>::max()) {

    // Log to verify the tecCheckboxes map
    for (const auto& entry : tecCheckboxes_) {
        wxLogMessage("TEC Label: %s, IsChecked: %d", entry.first, entry.second->IsChecked());
    }


    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}


void GraphPlotting::AddDataPoint(const std::vector<float>& currents,
    const std::vector<float>& voltages,
    const std::vector<std::string>& currentLabels,
    const std::vector<std::string>& voltageLabels,
    const wxString& time) {



    // Add filtered data and labels
    currentData_.push_back(currents);
    voltageData_.push_back(voltages);

    // Push each label individually
    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label);  // Push current labels individually
    }
    for (const auto& label : voltageLabels) {
        voltageLabels_.push_back(label);  // Push voltage labels individually
    }

    timeData_.push_back(time);

    // Update the min and max for currents and voltages
    for (auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    for (auto& voltage : voltages) {
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }


    RefreshGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents,
    const std::vector<std::string>& currentLabels,
    const wxString& time) {



    // Add  current data
    currentData_.push_back(currents);

    // Push each label individually
    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label);  // Push current labels individually
    }

    timeData_.push_back(time);

    // Update the min and max for currents
    for (const auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    RefreshGraph();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Add Method to Handle Only Voltages
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void GraphPlotting::render(wxDC& dc) {
    dc.Clear();  // Clear the canvas before drawing

    int width, height;
    this->GetSize(&width, &height);  // Get the dimensions of the window

    if (currentData_.empty() && voltageData_.empty()) return;

    // Dynamic range based on actual data
    float currentRange = (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_);
    float voltageRange = (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_);

    // Ensure the scaling factors take into account both positive and negative values
    float yScaleCurrent = (height - 100) / currentRange;
    float yScaleVoltage = (height - 100) / voltageRange;
    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);

    // Draw the outer rectangle for the graph (graph box)
    dc.SetPen(wxPen(*wxBLACK, 2));  // Thicker black border for the graph
    dc.DrawRectangle(50, 50, width - 100, height - 100);  // Create a box with padding

    // Draw the grid lines (lighter lines inside the graph)
    dc.SetPen(wxPen(wxColour(200, 200, 200), 1));  // Lighter grid lines
    for (int i = 0; i <= 10; i++) {
        int yPos = height - 50 - (i * (height - 100) / 10);  // Adjust the Y-axis grid
        dc.DrawLine(50, yPos, width - 50, yPos);  // Draw from left to right
    }


    //drawGridLines(dc, width, height);
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

    // Ensure we have valid data before accessing it
   // Ensure we have valid data before accessing it
    if (!currentData_.empty()) {
        size_t tecCount = currentData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            // Plot current values if available for this TEC
            if (tec < currentData_.front().size()) {  // Ensure the TEC data exists
                dc.SetPen(wxPen(wxColour(255, 0, 0), 2));  // Red line for current
                for (size_t i = 1; i < currentData_.size(); ++i) {
                    if (tec < currentData_[i - 1].size() && tec < currentData_[i].size()) {  // Check the size for safety
                        int x1 = static_cast<int>((i - 1) * xStep + 50);
                        int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);
                        int x2 = static_cast<int>(i * xStep + 50);
                        int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);
                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }
        }
    }

    // Plot voltage values similarly
    if (!voltageData_.empty()) {
        size_t tecCount = voltageData_.front().size();  // Number of TECs

        for (size_t tec = 0; tec < tecCount; ++tec) {
            // Plot voltage values if available for this TEC
            if (tec < voltageData_.front().size()) {
                dc.SetPen(wxPen(wxColour(0, 255, 0), 2));  // Green line for voltage
                for (size_t i = 1; i < voltageData_.size(); ++i) {
                    if (tec < voltageData_[i - 1].size() && tec < voltageData_[i].size()) {  // Check the size for safety
                        int x1 = static_cast<int>((i - 1) * xStep + 50);
                        int x2 = static_cast<int>(i * xStep + 50);
                        int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);
                        int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);

                        dc.DrawLine(x1, y1, x2, y2);
                    }
                }
            }
        }
    }

    // Draw axes labels and legend
   // drawAxesLabels(dc, width, height);
   // drawLegend(dc, width);
}




void GraphPlotting::drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit) {
    // Move the Y-axis labels a bit more to the left
    int xPos = leftAxis ? 25 : width - 30;  // Move left side labels further to xPos = 25
    const float maxDisplayValue = 1000;  // Cap for max value displayed on Y-axis
    float displayMaxValue = std::min(maxDisplayValue, maxValue);  // Clamp the max value

    // Ensure the Y-axis labels extend properly from the bottom to the top
    for (int i = 0; i <= 10; i++) {
        // Calculate the current label value, evenly spaced across the graph's height
        float value = minValue + i * (displayMaxValue - minValue) / 10;

        // Create a formatted label with the value and unit (A for current, V for voltage)
        wxString label = wxString::Format("%.2f %s", value, unit);

        // Correctly position the label along the Y-axis, ensuring the top-most label is drawn at the top
        int yPos = height - 50 - (i * (height - 100) / 10);  // Adjusted scaling to fit better within the graph's area
        dc.DrawText(label, wxPoint(xPos, yPos));
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