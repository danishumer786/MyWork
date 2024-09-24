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

    // Method for adding both currents and voltages, with labels
    void AddDataPoint(const std::vector<float>& currents,
        const std::vector<float>& voltages,
        const std::vector<std::string>& currentLabels,
        const std::vector<std::string>& voltageLabels,
        const wxString& time);

    // Method for adding only currents, with labels
    void AddCurrentDataPoint(const std::vector<float>& currents,
        const std::vector<std::string>& currentLabels,
        const wxString& time);

    // Method for adding only voltages, with labels
    void AddVoltageDataPoint(const std::vector<float>& voltages,
        const std::vector<std::string>& voltageLabels,
        const wxString& time);

    // Method for adding temperature data, with labels
    void AddTemperatureDataPoint(const std::vector<float>& temperatures,
        const std::vector<std::string>& tempLabels,
        const wxString& time);

    // Refresh the graph to reflect the updated data
    void RefreshGraph();

private:
    // Data storage for the graph
    std::deque<std::vector<float>> currentData_;        // Current data over time
    std::deque<std::vector<float>> voltageData_;        // Voltage data over time
    std::deque<std::vector<float>> temperatureData_;    // Temperature data over time
    std::vector<std::string> currentLabels_;            // Labels for current
    std::vector<std::string> voltageLabels_;            // Labels for voltage
    std::vector<std::string> tempLabels_;               // Labels for temperature
    std::deque<wxString> timeData_;                     // Time data

    // Min and max values for scaling
    float currentMax_ = std::numeric_limits<float>::lowest();
    float currentMin_ = std::numeric_limits<float>::max();
    float voltageMax_ = std::numeric_limits<float>::lowest();
    float voltageMin_ = std::numeric_limits<float>::max();
    float tempMax_ = std::numeric_limits<float>::lowest();
    float tempMin_ = std::numeric_limits<float>::max();

    // Vector to store the checkboxes for each TEC
    std::vector<wxCheckBox*> checkboxes_;

    // Event handling and rendering
    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);

    // Methods for drawing different components of the graph
    void OnResize(wxSizeEvent& event);

    // Method for drawing Y-axis labels
    void drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit);

    DECLARE_EVENT_TABLE();
};
