#pragma once

#include <wx/wx.h>
#include <vector>
#include <deque>
#include <set>
#include <map>  // For std::map

class GraphPlotting : public wxPanel {
public:
    // Constructor that accepts the checkboxes vector
    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, const std::vector<wxCheckBox*>& checkboxes);

    // Original method for adding both currents and voltages, with labels
    void AddDataPoint(const std::vector<float>& currents,
        const std::vector<float>& voltages,
        const std::vector<std::string>& currentLabels,
        const std::vector<std::string>& voltageLabels,
        const wxString& time);

    // New methods for adding only currents or only voltages, with labels
    void AddCurrentDataPoint(const std::vector<float>& currents,
        const std::vector<std::string>& currentLabels,
        const wxString& time);

    void AddVoltageDataPoint(const std::vector<float>& voltages,
        const std::vector<std::string>& voltageLabels,
        const wxString& time);

    // Refresh the graph to reflect the updated data
    void RefreshGraph();

private:
    // Data storage for the graph
    std::deque<std::vector<float>> currentData_;
    std::deque<std::vector<float>> voltageData_;
    std::vector<std::string> currentLabels_;
    std::vector<std::string> voltageLabels_;

    std::deque<wxString> timeData_;

    // Vector to store the checkboxes for each TEC
    std::vector<wxCheckBox*> checkboxes_;  // Corrected type: vector of checkboxes
    
    // Members to track the maximum and minimum values for scaling
    float currentMax_ = std::numeric_limits<float>::lowest();
    float currentMin_ = std::numeric_limits<float>::max();
    float voltageMax_ = std::numeric_limits<float>::lowest();
    float voltageMin_ = std::numeric_limits<float>::max();

    // Event handling and rendering
    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);

    // Methods for drawing different components of the graph
    // void drawAxesLabels(wxDC& dc, int width, int height);
    // void drawLegend(wxDC& dc, int width);
    // void drawGridLines(wxDC& dc, int width, int height);
    void OnResize(wxSizeEvent& event);

    // Method for drawing Y-axis labels
    void drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit);

    DECLARE_EVENT_TABLE();
};
