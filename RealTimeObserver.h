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
    // TEC-related members
    wxPanel* currentPanel_;
    wxPanel* voltagePanel_;
    wxPanel* tempPanel_;
    wxBitmapButton* currentToggleButton_;
    wxBitmapButton* voltageToggleButton_;
    wxBitmapButton* tempToggleButton_;
    wxPanel* tecContainer_;
    wxPanel* mainTecTogglePanel_;
    wxBitmapButton* tecPanelToggleButton_;
    std::function<void()> updateLayout_;
    wxBitmap expandBitmap;
    wxBitmap collapseBitmap;

    // Diode, Power, and Sensor toggle panels
    wxPanel* diodeTogglePanel_ ;
    wxPanel* powerTogglePanel_ ;
    wxPanel* sensorTogglePanel_ ;


    // Diode, Power, Sensor related members
    wxPanel* diodeContentPanel_;
    wxBitmapButton* diodeToggleButton_;

    wxPanel* powerContentPanel_;
    wxBitmapButton* powerToggleButton_;

    wxPanel* sensorContentPanel_;
    wxBitmapButton* sensorToggleButton_;

    // Alarm and logging controls
    wxString alarmMessage_;
    wxString alarmTime_;
    wxString lastAlarmMessage_;
    wxString lastAlarmTime_;
    wxTextCtrl* textCtrl_;
    wxTextCtrl* alarmTextCtrl_;
    bool alarmTriggered_;

    // Plot references
    GraphPlotting* currentPlot_ = nullptr;
    GraphPlotting* voltagePlot_ = nullptr;
    GraphPlotting* tempPlot_ = nullptr;
    GraphPlotting* diodePlot_ = nullptr;
    GraphPlotting* powerPlot_ = nullptr;
    GraphPlotting* sensorPlot_ = nullptr;

    PlotType observerType_;

public:
    std::vector<std::string> alarms;

    std::vector<float> currents;
    std::vector<float> voltages;
    std::vector<float> temperatures;
    std::vector<float> diodeCurrents;
    std::vector<float> powerReadings;
    std::vector<float> sensorReadings;
    std::vector<std::string> sensorLabels;

    std::vector<std::string> currentLabels;
    std::vector<std::string> voltageLabels;
    std::vector<std::string> tempLabels;
    std::vector<std::string> diodeCurrentLabels;
    std::vector<std::string> powerLabels;

    bool currentDataFound = false;
    bool voltageDataFound = false;
    bool tempDataFound = false;
    bool diodeCurrentDataFound = false;
    bool powerDataFound = false;
    bool sensorDataFound = false;
    bool alarmDataFound = false;
    //PlotType observerType_;

    // Constructor for monitoring real-time data and alarm updates
    RealTimeObserver(wxTextCtrl* textCtrl, wxTextCtrl* alarmTextCtrl = nullptr)
        : textCtrl_(textCtrl), alarmTextCtrl_(alarmTextCtrl), alarmTriggered_(false) {}

    // Constructor for Diode, Power, and Sensor plots
    RealTimeObserver(wxTextCtrl* textCtrl, GraphPlotting* plot, PlotType plotType,
        wxPanel* contentPanel, wxBitmapButton* toggleButton, wxPanel* togglePanel,
        std::function<void()> updateLayout = nullptr)
        : textCtrl_(textCtrl), updateLayout_(updateLayout), alarmTriggered_(false) {

        if (plotType == PlotType::Diode) {
            diodePlot_ = plot;
            diodeContentPanel_ = contentPanel;
            diodeToggleButton_ = toggleButton;
            diodeTogglePanel_ = togglePanel;
        }
        else if (plotType == PlotType::Power) {
            powerPlot_ = plot;
            powerContentPanel_ = contentPanel;
            powerToggleButton_ = toggleButton;
            powerTogglePanel_ = togglePanel;
        }
        else if (plotType == PlotType::Sensors) {
            sensorPlot_ = plot;
            sensorContentPanel_ = contentPanel;
            sensorToggleButton_ = toggleButton;
            sensorTogglePanel_ = togglePanel;
        }
    }



    // Constructor for TEC plots (current, voltage, temperature)
    RealTimeObserver(wxTextCtrl* textCtrl,
        GraphPlotting* currentPlot, GraphPlotting* voltagePlot, GraphPlotting* tempPlot,
        wxPanel* currentPanel, wxBitmapButton* currentToggleButton,
        wxPanel* voltagePanel, wxBitmapButton* voltageToggleButton,
        wxPanel* tempPanel, wxBitmapButton* tempToggleButton,
        wxPanel* tecContainer, wxPanel* mainTecTogglePanel,
        wxBitmapButton* tecPanelToggleButton, wxBitmap expandBitmap,
        wxBitmap collapseBitmap, std::function<void()> updateLayout)
        : textCtrl_(textCtrl), currentPlot_(currentPlot), voltagePlot_(voltagePlot),
        tempPlot_(tempPlot), currentPanel_(currentPanel), currentToggleButton_(currentToggleButton),
        voltagePanel_(voltagePanel), voltageToggleButton_(voltageToggleButton),
        tempPanel_(tempPanel), tempToggleButton_(tempToggleButton),
        tecContainer_(tecContainer), mainTecTogglePanel_(mainTecTogglePanel),
        tecPanelToggleButton_(tecPanelToggleButton), expandBitmap(expandBitmap),
        collapseBitmap(collapseBitmap), updateLayout_(updateLayout), alarmTriggered_(false) {}

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
