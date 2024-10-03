#pragma once

#include <wx/wx.h>
#include <vector>
#include <deque>
#include <set>
#include <map>  

class GraphPlotting : public wxPanel {
public:

    GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, const std::vector<wxCheckBox*>& checkboxes);

    void AddDataPoint(const std::vector<float>& currents, const std::vector<float>& voltages, const std::vector<std::string>& currentLabels, const std::vector<std::string>& voltageLabels, const wxString& time);
    void AddCurrentDataPoint(const std::vector<float>& currents, const std::vector<std::string>& currentLabels, const wxString& time);
    void AddVoltageDataPoint(const std::vector<float>& voltages, const std::vector<std::string>& voltageLabels, const wxString& time);
    void AddTemperatureDataPoint(const std::vector<float>& temperatures, const std::vector<std::string>& tempLabels, const wxString& time);
    void AddDiodeCurrentDataPoint(const std::vector<float>& currents, const std::vector<std::string>& labels, const wxString& time);
    void AddPowerDataPoint(const std::vector<float>& powerReadings, const std::vector<std::string>& labels, const wxString& time);
    void AddSensorDataPoint(const std::vector<float>& sensorReadings, const std::vector<std::string>& labels, const wxString& time);
    void RefreshGraph();

   

    void SetAlarmTriggered(bool alarmTriggered, const wxString& alarmMessage = "", const wxString& alarmTime = "") {
        alarmTriggered_ = alarmTriggered;
        if (alarmTriggered) {
            alarmMessage_ = alarmMessage;  // Store the alarm message
            alarmTime_ = alarmTime;        // Store the alarm time
        }
        else {
            alarmMessage_.Clear();
            alarmTime_.Clear();
        }
    }


private:
    bool alarmTriggered_ = false;

    wxString alarmMessage_;  // Store the alarm message
    wxString alarmTime_;

    static constexpr int maxDataPoints_ = 1000;

    std::vector<std::vector<float>> diodeCurrentData_;
    std::vector<std::string> diodeCurrentLabels_;

    std::deque<std::vector<float>> currentData_;
    std::vector<std::string> currentLabels_;

    std::deque<std::vector<float>> voltageData_;
    std::vector<std::string> voltageLabels_;

    std::deque<std::vector<float>> temperatureData_;
    std::vector<std::string> tempLabels_;

    std::deque<std::vector<float>> powerData_;
    std::vector<std::string> powerLabels_;

    std::deque<std::vector<float>> sensorData_;
    std::vector<std::string> sensorLabels_;



    std::deque<wxString> timeData_;


    float currentMax_ = std::numeric_limits<float>::lowest();
    float currentMin_ = std::numeric_limits<float>::max();
    float voltageMax_ = std::numeric_limits<float>::lowest();
    float voltageMin_ = std::numeric_limits<float>::max();
    float tempMax_ = std::numeric_limits<float>::lowest();
    float tempMin_ = std::numeric_limits<float>::max();
    float diodeCurrentMax_ = std::numeric_limits<float>::lowest();
    float diodeCurrentMin_ = std::numeric_limits<float>::max();
    float powerMax_ = std::numeric_limits<float>::lowest();
    float powerMin_ = std::numeric_limits<float>::max();
    float sensorMax_ = std::numeric_limits<float>::lowest();
    float sensorMin_ = std::numeric_limits<float>::max();


    std::vector<wxCheckBox*> checkboxes_;


    void paintEvent(wxPaintEvent& evt);
    void render(wxDC& dc);
    void OnResize(wxSizeEvent& event);
    void drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit);

    DECLARE_EVENT_TABLE();
};
