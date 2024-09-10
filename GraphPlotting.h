#pragma once

#include <wx/wx.h>
#include <vector>
#include <deque>

class GraphPlotting : public wxPanel {
public:
    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size);

    // Update to accept multiple TEC's currents and voltages
    void AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time);
    void RefreshGraph();

private:
    std::deque<std::vector<float>> currentData_;  // Store current data for multiple TECs
    std::deque<std::vector<float>> voltageData_;  // Store voltage data for multiple TECs
    std::deque<wxString> timeData_;               // Store corresponding time values

    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);
    void drawAxesLabels(wxDC& dc, int width, int height);
    void drawLegend(wxDC& dc, int width);
    void drawGridLines(wxDC& dc, int width, int height);
    void OnResize(wxSizeEvent& event);

    DECLARE_EVENT_TABLE();
};
