#pragma once

#include <wx/wx.h>
#include <vector>
#include <deque>

class GraphPlotting : public wxPanel {
public:
    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size);

    void AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time);
    void RefreshGraph();

private:
    std::deque<std::vector<float>> currentData_;
    std::deque<std::vector<float>> voltageData_;
    std::deque<wxString> timeData_;

    // Newly added members
    float currentMax_, currentMin_, voltageMax_, voltageMin_;

    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);
    void drawAxesLabels(wxDC& dc, int width, int height);
    void drawLegend(wxDC& dc, int width);
    void drawGridLines(wxDC& dc, int width, int height);
    void OnResize(wxSizeEvent& event);
    void drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float yScale, float minValue, const wxString& unit);

    DECLARE_EVENT_TABLE();
};
