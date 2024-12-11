#pragma once
#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"
#include "GraphPlotting.h"
#include <map>
#include <vector>

// Enum to differentiate between different plot types
enum class PlotType { Diode, Power, TEC, Sensors };  


class RealTimeObserver : public LogObserver {
private:
    wxString alarmMessage_;
    wxString alarmTime_;
    wxString lastAlarmMessage_;
    wxString lastAlarmTime_;
    wxTextCtrl* textCtrl_;
    wxTextCtrl* alarmTextCtrl_;
    GraphPlotting* currentPlot_ = nullptr;
    GraphPlotting* voltagePlot_ = nullptr;
    GraphPlotting* tempPlot_ = nullptr;
    GraphPlotting* diodePlot_ = nullptr;
    GraphPlotting* powerPlot_ = nullptr;
    GraphPlotting* sensorPlot_ = nullptr;
    bool alarmTriggered_;

public:
    // Constructor for TEC plots (current, voltage, temperature)
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* currentPlot, GraphPlotting* voltagePlot, GraphPlotting* tempPlot)
        : textCtrl_(textCtrl), currentPlot_(currentPlot), voltagePlot_(voltagePlot), tempPlot_(tempPlot) {}

    RealTimeObserver(wxTextCtrl* textCtrl, wxTextCtrl* alarmTextCtrl = nullptr)
        : textCtrl_(textCtrl), alarmTextCtrl_(alarmTextCtrl), alarmTriggered_(false) {}

    // Constructor for Diode, Power, and Sensor plots
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* plot, PlotType plotType)
        : textCtrl_(textCtrl) {
       
        if (plotType == PlotType::Diode) {
            diodePlot_ = plot;
        }
        else if (plotType == PlotType::Power) {
            powerPlot_ = plot;
        }
        else if (plotType == PlotType::Sensors) {
            sensorPlot_ = plot; 
        }
    }
    // Method to set the alarm text control (from the graph window)
    void SetAlarmTextCtrl(wxTextCtrl* alarmTextCtrl) {
        alarmTextCtrl_ = alarmTextCtrl;
    }
    bool IsAlarmTriggered() const {
        return alarmTriggered_;
    }

    // Method to handle new data points
    void onDataPointLogged(std::map<std::string, std::string> data) override;
};
