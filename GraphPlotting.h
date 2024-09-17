#pragma once

#include <wx/wx.h>
#include <vector>
#include <deque>
#include <set>

class GraphPlotting : public wxPanel {
public:
    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size);

    // Original method for adding both currents and voltages
    void AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time);

    // New methods for adding only currents or only voltages
    void AddCurrentDataPoint(const std::vector<float>& currents, const wxString& time);
    void AddVoltageDataPoint(const std::vector<float>& voltages, const wxString& time);



    void RefreshGraph();

private:
    std::deque<std::vector<float>> currentData_;
    std::deque<std::vector<float>> voltageData_;
    std::deque<wxString> timeData_;


    // Members to track the maximum and minimum values for scaling
    float currentMax_, currentMin_, voltageMax_, voltageMin_;

    // Event handling and rendering
    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);

    // Methods for drawing different components of the graph
    void drawAxesLabels(wxDC& dc, int width, int height);
    void drawLegend(wxDC& dc, int width);
    void drawGridLines(wxDC& dc, int width, int height);
    void OnResize(wxSizeEvent& event);

    // Method for drawing Y-axis labels
    void drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit);

    DECLARE_EVENT_TABLE();
};
