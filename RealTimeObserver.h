#pragma once
#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"
#include <map>
#include <vector>

class RealTimeObserver : public LogObserver {
public:
    // Constructor for TEC plots (current, voltage, temperature)
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* currentPlot, GraphPlotting* voltagePlot, GraphPlotting* tempPlot)
        : textCtrl_(textCtrl), currentPlot_(currentPlot), voltagePlot_(voltagePlot), tempPlot_(tempPlot) {}

    // Constructor for Diode plot only
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* diodePlot)
        : textCtrl_(textCtrl), diodePlot_(diodePlot) {}

    // Method to handle new data points
    void onDataPointLogged(std::map<std::string, std::string> data) override;

private:
    wxTextCtrl* textCtrl_;
    GraphPlotting* currentPlot_ = nullptr;
    GraphPlotting* voltagePlot_ = nullptr;
    GraphPlotting* tempPlot_ = nullptr;
    GraphPlotting* diodePlot_ = nullptr;  // Added diodePlot for handling diode current plotting
};
