#pragma once
#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"
#include <map>
#include <vector>

// Enum to differentiate between different plot types
enum class PlotType { Diode, Power, TEC, Sensors };  // Add Sensors to the enum


class RealTimeObserver : public LogObserver {
public:
    // Constructor for TEC plots (current, voltage, temperature)
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* currentPlot, GraphPlotting* voltagePlot, GraphPlotting* tempPlot)
        : textCtrl_(textCtrl), currentPlot_(currentPlot), voltagePlot_(voltagePlot), tempPlot_(tempPlot) {}

    RealTimeObserver(wxTextCtrl* textCtrl, wxTextCtrl* alarmTextCtrl = nullptr)
        : textCtrl_(textCtrl), alarmTextCtrl_(alarmTextCtrl) {}

    // Constructor for Diode, Power, and Sensor plots
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* plot, PlotType plotType)
        : textCtrl_(textCtrl) {
        // Determine which plot to assign based on the PlotType enum
        if (plotType == PlotType::Diode) {
            diodePlot_ = plot;
        }
        else if (plotType == PlotType::Power) {
            powerPlot_ = plot;
        }
        else if (plotType == PlotType::Sensors) {
            sensorPlot_ = plot;  // Assign the sensor plot
        }
    }
    // Method to set the alarm text control (from the graph window)
    void SetAlarmTextCtrl(wxTextCtrl* alarmTextCtrl) {
        alarmTextCtrl_ = alarmTextCtrl;
    }

    // Method to handle new data points
    void onDataPointLogged(std::map<std::string, std::string> data) override;


private:
    wxTextCtrl* textCtrl_;
    wxTextCtrl* alarmTextCtrl_;
    GraphPlotting* currentPlot_ = nullptr;
    GraphPlotting* voltagePlot_ = nullptr;
    GraphPlotting* tempPlot_ = nullptr;
    GraphPlotting* diodePlot_ = nullptr;
    GraphPlotting* powerPlot_ = nullptr;  // Added powerPlot for handling power data plotting
    GraphPlotting* sensorPlot_ = nullptr;
    //bool& alarmFlag_;
};
