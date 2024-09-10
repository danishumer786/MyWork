#include "GraphPlotting.h"
#include "wx/dcbuffer.h"
#include <algorithm>
#include <limits>

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
    EVT_PAINT(GraphPlotting::paintEvent)
    EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, winid, pos, size) {
    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void GraphPlotting::AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time) {
    // Store current and voltage data for multiple TECs
    currentData_.push_back(currents);
    voltageData_.push_back(voltages);
    timeData_.push_back(time);

    // Keep a limited number of data points for visualization
    const int maxVisiblePoints = 50;  // Adjust this to fit the graph window
    if (timeData_.size() > maxVisiblePoints) {
        timeData_.erase(timeData_.begin());
        currentData_.erase(currentData_.begin());
        voltageData_.erase(voltageData_.begin());
    }

    // Refresh the graph after adding a new data point
    RefreshGraph();
}

void GraphPlotting::render(wxDC& dc) {
    dc.Clear();  // Clear the canvas before drawing

    if (currentData_.empty() && voltageData_.empty()) {
        return;
    }

    int width, height;
    this->GetSize(&width, &height);  // Get the dimensions of the window

    // Calculate scaling factors for the Y-axis
    float maxValue = std::numeric_limits<float>::min();
    float minValue = std::numeric_limits<float>::max();

    // Calculate the min/max values for both current and voltage
    for (const auto& currents : currentData_) {
        if (!currents.empty()) {
            maxValue = std::max(maxValue, *std::max_element(currents.begin(), currents.end()));
            minValue = std::min(minValue, *std::min_element(currents.begin(), currents.end()));
        }
    }

    for (const auto& voltages : voltageData_) {
        if (!voltages.empty()) {
            maxValue = std::max(maxValue, *std::max_element(voltages.begin(), voltages.end()));
            minValue = std::min(minValue, *std::min_element(voltages.begin(), voltages.end()));
        }
    }

    // Make sure minValue and maxValue are not equal to avoid division by zero
    if (maxValue == minValue) {
        maxValue += 1.0f;  // Add a small buffer to avoid zero division
    }

    // Calculate scaling factors for plotting
    float yScale = static_cast<float>(height - 50) / (maxValue - minValue); // Y-axis scaling based on data range
    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);  // X-axis scaling based on number of points

    // Draw Y-axis labels
    for (int i = 0; i <= 10; ++i) {
        int yPos = height - 50 - static_cast<int>((i * (height - 50)) / 10);
        float value = minValue + i * (maxValue - minValue) / 10;
        dc.DrawText(wxString::Format("%.2f", value), wxPoint(10, yPos));
    }
    // Define colors for TECs
    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),    // Red for TEC 1 Current
        wxColour(0, 255, 0),    // Green for TEC 1 Voltage
        wxColour(0, 0, 255),    // Blue for TEC 2 Current
        wxColour(255, 255, 0),  // Yellow for TEC 2 Voltage
        wxColour(0, 255, 255),  // Cyan for TEC 3 Current (if needed)
        wxColour(255, 0, 255)   // Magenta for TEC 3 Voltage (if needed)
    };

    // Draw X-axis labels and grid lines
    if (!timeData_.empty() && timeData_.size() > 1) {
        size_t maxLabels = 10;
        size_t labelInterval = std::max(1, static_cast<int>(timeData_.size() / maxLabels));

        for (size_t i = 0; i < timeData_.size(); i += labelInterval) {
            int x = static_cast<int>(i * (width - 100) / (timeData_.size() - 1));

            // Draw vertical grid lines
            dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));  // Light gray dotted lines for grid
            dc.DrawLine(x + 50, 0, x + 50, height - 50);

            // Label the X-axis
            dc.SetPen(wxPen(wxColour(0, 0, 0), 1));  // Black for text
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));  // Smaller font size
            dc.DrawText(timeData_[i], wxPoint(x + 45, height - 40));
        }
    }

    // Plot current values for each TEC
    for (size_t tec = 0; tec < currentData_.front().size(); ++tec) {
        dc.SetPen(wxPen(colors[tec * 2 % colors.size()], 2));  // Use different colors for each TEC current
        for (size_t i = 1; i < currentData_.size(); ++i) {
            int x1 = static_cast<int>((i - 1) * xStep + 50);
            int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - minValue) * yScale);
            int x2 = static_cast<int>(i * xStep + 50);
            int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - minValue) * yScale);
            dc.DrawLine(x1, y1, x2, y2);
        }
    }

    // Plot voltage values for each TEC
    for (size_t tec = 0; tec < voltageData_.front().size(); ++tec) {
        dc.SetPen(wxPen(colors[(tec * 2 + 1) % colors.size()], 2));  // Use different colors for each TEC voltage
        for (size_t i = 1; i < voltageData_.size(); ++i) {
            int x1 = static_cast<int>((i - 1) * xStep + 50);
            int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - minValue) * yScale);
            int x2 = static_cast<int>(i * xStep + 50);
            int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - minValue) * yScale);
            dc.DrawLine(x1, y1, x2, y2);
        }
    }

    // Draw grid lines
    drawGridLines(dc, width, height);

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

    // Vertical grid lines (X-axis)
    if (!timeData_.empty() && timeData_.size() > 1) {  // Ensure timeData_ has more than one data point
        for (size_t i = 0; i < timeData_.size(); i += std::max(1, static_cast<int>(timeData_.size() / 10))) {
            int x = static_cast<int>(i * (width - 100) / (timeData_.size() - 1));  // Calculate X position
            dc.DrawLine(x + 50, 0, x + 50, height - 50);  // Draw vertical lines
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
    for (size_t tec = 0; tec < currentData_.front().size(); ++tec) {
        // Current
        dc.SetPen(wxPen(colors[tec * 2 % colors.size()], 2));  // Multiply index by 2 for current
        dc.DrawLine(width - 100, legendYPosition, width - 80, legendYPosition);
        dc.DrawText(wxString::Format("TEC %d Current", tec + 1), wxPoint(width - 75, legendYPosition - 5));
        legendYPosition += 20;

        // Voltage
        dc.SetPen(wxPen(colors[(tec * 2 + 1) % colors.size()], 2));  // Offset by 1 for voltage
        dc.DrawLine(width - 100, legendYPosition, width - 80, legendYPosition);
        dc.DrawText(wxString::Format("TEC %d Voltage", tec + 1), wxPoint(width - 75, legendYPosition - 5));
        legendYPosition += 20;
    }
}

void GraphPlotting::drawAxesLabels(wxDC& dc, int width, int height) {
    // Use a smaller font size to further reduce density
    wxFont font(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);

    // Move the Y-axis label (Voltage/Current) more to the center horizontally
    dc.DrawText("Current/Voltage (A/V)", wxPoint(200, height - 150));  // Adjusted X and Y position

    // X-axis label remains centered horizontally
    dc.DrawText("Time (hh:mm:ss)", wxPoint(width / 2 - 50, height + 20));
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
