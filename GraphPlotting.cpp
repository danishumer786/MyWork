#include "GraphPlotting.h"
#include "wx/dcbuffer.h"

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
EVT_PAINT(GraphPlotting::paintEvent)
EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, winid, pos, size) {
    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void GraphPlotting::AddDataPoint(float tec1Current, float tec2Current, float tec1Voltage, float tec2Voltage, const wxString& time) {
    tec1CurrentData_.push_back(tec1Current);
    tec2CurrentData_.push_back(tec2Current);
    tec1VoltageData_.push_back(tec1Voltage);
    tec2VoltageData_.push_back(tec2Voltage);
    timeData_.push_back(time);

    const int maxVisibleLabels = 10;
    if (timeData_.size() > maxVisibleLabels) {
        timeData_.erase(timeData_.begin());
        tec1CurrentData_.erase(tec1CurrentData_.begin());
        tec2CurrentData_.erase(tec2CurrentData_.begin());
        tec1VoltageData_.erase(tec1VoltageData_.begin());
        tec2VoltageData_.erase(tec2VoltageData_.begin());
    }

    RefreshGraph();  // Update the graph
}

void GraphPlotting::RefreshGraph() {
    this->Refresh();
    this->Update();
}

void GraphPlotting::paintEvent(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    render(dc);
}

void GraphPlotting::render(wxDC& dc) {
    dc.Clear();  // Clear the canvas before drawing

    if (tec1CurrentData_.empty() || tec2CurrentData_.empty() || tec1VoltageData_.empty() || tec2VoltageData_.empty()) {
        wxLogMessage("No data to plot");
        return;
    }

    int width, height;
    this->GetSize(&width, &height);

    // Calculate scaling factors
    float maxValue = std::max({
        *std::max_element(tec1CurrentData_.begin(), tec1CurrentData_.end()),
        *std::max_element(tec2CurrentData_.begin(), tec2CurrentData_.end()),
        *std::max_element(tec1VoltageData_.begin(), tec1VoltageData_.end()),
        *std::max_element(tec2VoltageData_.begin(), tec2VoltageData_.end())
        });
    float minValue = std::min({
        *std::min_element(tec1CurrentData_.begin(), tec1CurrentData_.end()),
        *std::min_element(tec2CurrentData_.begin(), tec2CurrentData_.end()),
        *std::min_element(tec1VoltageData_.begin(), tec1VoltageData_.end()),
        *std::min_element(tec2VoltageData_.begin(), tec2VoltageData_.end())
        });

    // Add a margin to avoid lines touching the top and bottom
    float margin = (maxValue - minValue) * 0.1f;
    maxValue += margin;
    minValue -= margin;

    float valueRange = maxValue - minValue;
    if (valueRange == 0) valueRange = 1;  // Prevent division by zero

    float xStep = static_cast<float>(width - 100) / (tec1CurrentData_.size() - 1);  // Leave margin for axes
    float yScale = static_cast<float>(height - 50) / valueRange;  // Leave space for labels

    // Draw the Y-axis (left side)
    dc.SetPen(wxPen(wxColour(0, 0, 0), 2)); // Black pen for axes
    dc.DrawLine(50, 0, 50, height - 50);  // Vertical Y-axis

    // Draw the X-axis (bottom side)
    dc.DrawLine(50, height - 50, width, height - 50);  // Horizontal X-axis

    // Label the Y-axis (current and voltage values)
    int yAxisSteps = 10; // Number of steps on the Y-axis
    float yStepValue = valueRange / yAxisSteps;  // Value increment per step
    for (int i = 0; i <= yAxisSteps; ++i) {
        int yPos = height - 50 - static_cast<int>((i * (height - 50)) / yAxisSteps);
        float value = minValue + i * yStepValue;
        dc.DrawText(wxString::Format("%.1f", value), wxPoint(10, yPos));  // Display values on the left
    }

    // Label the X-axis (time values)
    for (size_t i = 0; i < timeData_.size(); i += 5) {  // Label every 5th point
        int x = static_cast<int>(i * xStep);
        dc.DrawText(timeData_[i], wxPoint(x + 50, height - 40));  // Adjust position for time labels
    }

    // Draw lines for TEC1 current
    dc.SetPen(wxPen(wxColour(255, 0, 0), 2)); // Red pen for TEC1 current
    for (size_t i = 1; i < tec1CurrentData_.size(); ++i) {
        int x1 = static_cast<int>((i - 1) * xStep + 50);  // Start from margin
        int y1 = height - 50 - static_cast<int>((tec1CurrentData_[i - 1] - minValue) * yScale);
        int x2 = static_cast<int>(i * xStep + 50);
        int y2 = height - 50 - static_cast<int>((tec1CurrentData_[i] - minValue) * yScale);

        dc.DrawLine(x1, y1, x2, y2);
    }

    // Draw lines for TEC2 current
    dc.SetPen(wxPen(wxColour(0, 255, 0), 2)); // Green pen for TEC2 current
    for (size_t i = 1; i < tec2CurrentData_.size(); ++i) {
        int x1 = static_cast<int>((i - 1) * xStep + 50);
        int y1 = height - 50 - static_cast<int>((tec2CurrentData_[i - 1] - minValue) * yScale);
        int x2 = static_cast<int>(i * xStep + 50);
        int y2 = height - 50 - static_cast<int>((tec2CurrentData_[i] - minValue) * yScale);

        dc.DrawLine(x1, y1, x2, y2);
    }

    // Draw lines for TEC1 voltage
    dc.SetPen(wxPen(wxColour(0, 0, 255), 2)); // Blue pen for TEC1 voltage
    for (size_t i = 1; i < tec1VoltageData_.size(); ++i) {
        int x1 = static_cast<int>((i - 1) * xStep + 50);
        int y1 = height - 50 - static_cast<int>((tec1VoltageData_[i - 1] - minValue) * yScale);
        int x2 = static_cast<int>(i * xStep + 50);
        int y2 = height - 50 - static_cast<int>((tec1VoltageData_[i] - minValue) * yScale);

        dc.DrawLine(x1, y1, x2, y2);
    }

    // Draw lines for TEC2 voltage
    dc.SetPen(wxPen(wxColour(255, 255, 0), 2)); // Yellow pen for TEC2 voltage
    for (size_t i = 1; i < tec2VoltageData_.size(); ++i) {
        int x1 = static_cast<int>((i - 1) * xStep + 50);
        int y1 = height - 50 - static_cast<int>((tec2VoltageData_[i - 1] - minValue) * yScale);
        int x2 = static_cast<int>(i * xStep + 50);
        int y2 = height - 50 - static_cast<int>((tec2VoltageData_[i] - minValue) * yScale);

        dc.DrawLine(x1, y1, x2, y2);
    }

    // Draw Axes Labels
    drawAxesLabels(dc, width, height);

    // Draw Legend
    drawLegend(dc, width);
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

void GraphPlotting::drawLegend(wxDC& dc, int width) {
    // Add a simple legend to identify lines
    dc.SetPen(wxPen(wxColour(255, 0, 0), 2));  // Red for TEC1 current
    dc.DrawLine(width - 100, 20, width - 80, 20);
    dc.DrawText("TEC1 Current", wxPoint(width - 75, 15));

    dc.SetPen(wxPen(wxColour(0, 255, 0), 2));  // Green for TEC2 current
    dc.DrawLine(width - 100, 40, width - 80, 40);
    dc.DrawText("TEC2 Current", wxPoint(width - 75, 35));

    dc.SetPen(wxPen(wxColour(0, 0, 255), 2));  // Blue for TEC1 voltage
    dc.DrawLine(width - 100, 60, width - 80, 60);
    dc.DrawText("TEC1 Voltage", wxPoint(width - 75, 55));

    dc.SetPen(wxPen(wxColour(255, 255, 0), 2));  // Yellow for TEC2 voltage
    dc.DrawLine(width - 100, 80, width - 80, 80);
    dc.DrawText("TEC2 Voltage", wxPoint(width - 75, 75));
}

void GraphPlotting::OnResize(wxSizeEvent& event) {
    this->Refresh();
    event.Skip();
}
