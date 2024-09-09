#pragma once

#include <wx/wx.h>
#include <vector>
#include <algorithm>

class GraphPlotting : public wxPanel {
public:
    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size);

    void AddDataPoint(float tec1Current, float tec2Current, float tec1Voltage, float tec2Voltage, const wxString& time);
    void RefreshGraph();

private:
    std::vector<float> tec1CurrentData_;  // TEC1 current values
    std::vector<float> tec2CurrentData_;  // TEC2 current values
    std::vector<float> tec1VoltageData_;  // TEC1 voltage values
    std::vector<float> tec2VoltageData_;  // TEC2 voltage values
    std::vector<wxString> timeData_;      // Store corresponding time values

    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);
    void drawGridLines(wxDC& dc, int width, int height, float maxValue, float minValue, int labelInterval);
    void drawAxesLabels(wxDC& dc, int width, int height);
    void drawLegend(wxDC& dc, int width);
    void OnResize(wxSizeEvent& event);

    DECLARE_EVENT_TABLE();
};
