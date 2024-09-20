#include "GraphPlotting.h"
#include "wx/dcbuffer.h"
#include <algorithm>
#include <limits>

BEGIN_EVENT_TABLE(GraphPlotting, wxPanel)
EVT_PAINT(GraphPlotting::paintEvent)
EVT_SIZE(GraphPlotting::OnResize)
END_EVENT_TABLE()

GraphPlotting::GraphPlotting(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, const std::vector<wxCheckBox*>& checkboxes)
    : wxPanel(parent, winid, pos, size),
    checkboxes_(checkboxes), currentMax_(std::numeric_limits<float>::lowest()),
    currentMin_(std::numeric_limits<float>::max()),
    voltageMax_(std::numeric_limits<float>::lowest()),
    voltageMin_(std::numeric_limits<float>::max()) {  // Properly initializing vector of checkboxes

    this->SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
   
}
void GraphPlotting::AddDataPoint(const std::vector<float>& currents,
    const std::vector<float>& voltages,
    const std::vector<std::string>& currentLabels,
    const std::vector<std::string>& voltageLabels,
    const wxString& time) {
 
    currentData_.push_back(currents);
    voltageData_.push_back(voltages);

    
    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label); 
    }
    for (const auto& label : voltageLabels) {
        voltageLabels_.push_back(label); 
    }

    timeData_.push_back(time);


    // Update the min and max for currents and voltages
    for (auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    for (auto& voltage : voltages) {
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }



    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddCurrentDataPoint(const std::vector<float>& currents,
    const std::vector<std::string>& currentLabels,
    const wxString& time) {

    currentData_.push_back(currents);

    for (const auto& label : currentLabels) {
        currentLabels_.push_back(label);  // Push current labels individually
    }
    timeData_.push_back(time);

    for (const auto& current : currents) {
        if (current > currentMax_) currentMax_ = current;
        if (current < currentMin_) currentMin_ = current;
    }

    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::AddVoltageDataPoint(const std::vector<float>& voltages,
    const std::vector<std::string>& voltageLabels,
    const wxString& time) {



    // Add filtered voltage data
    voltageData_.push_back(voltages);

    // Push each label individually
    for (const auto& label : voltageLabels) {
        voltageLabels_.push_back(label);  // Push voltage labels individually
    }

    timeData_.push_back(time);

    // Update the min and max for voltages
    for (const auto& voltage : voltages) {
        if (voltage > voltageMax_) voltageMax_ = voltage;
        if (voltage < voltageMin_) voltageMin_ = voltage;
    }
    RefreshGraph();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------//
void GraphPlotting::render(wxDC& dc) {
    dc.Clear();  

    int width, height;
    this->GetSize(&width, &height);  

    if (currentData_.empty() && voltageData_.empty()) return;

    const float margin = 0.05;

    float currentRange = (currentMax_ - currentMin_) == 0 ? 1 : (currentMax_ - currentMin_);
    float voltageRange = (voltageMax_ - voltageMin_) == 0 ? 1 : (voltageMax_ - voltageMin_);

    
    float yScaleCurrent = (height - 100) / currentRange;
    float yScaleVoltage = (height - 100) / voltageRange;
    float xStep = static_cast<float>(width - 100) / (timeData_.size() - 1);

    dc.SetPen(wxPen(*wxBLACK, 2)); 
    dc.DrawRectangle(50, 50, width - 100, height - 100);  
    
    dc.SetPen(wxPen(wxColour(200, 200, 200), 1));  
    for (int i = 0; i <= 10; i++) {
        int yPos = height - 50 - (i * (height - 100) / 10); 
        dc.DrawLine(50, yPos, width - 50, yPos); 
    }
    dc.SetPen(wxPen(*wxBLACK, 1));  
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    int visibleLabels = 5;  
    int startIndex = 0;     
    int endIndex =(timeData_.size()); 

    if (timeData_.size() > visibleLabels) {
        startIndex = static_cast<int>(timeData_.size()) - visibleLabels;
    }
     xStep = static_cast<float>(width - 100) / (visibleLabels - 1);

    
    for (size_t i = startIndex; i < endIndex; ++i) {
        int xPos = static_cast<int>((i - startIndex) * xStep + 50); 
        wxString timeLabel = timeData_[i];  
        dc.DrawText(timeLabel, wxPoint(xPos - 10, height - 45)); 
    }

    drawYAxisLabels(dc, width, height, true, currentMax_, currentMin_, "A");  
    drawYAxisLabels(dc, width, height, false, voltageMax_, voltageMin_, "V");

    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),    // Red for TEC 1 Current
        wxColour(0, 255, 0),    // Green for TEC 1 Voltage
        wxColour(0, 0, 255),    // Blue for TEC 2 Current
        wxColour(255, 255, 0),  // Yellow for TEC 2 Voltage
        wxColour(0, 255, 255),  // Cyan for TEC 3 Current (if needed)
        wxColour(255, 0, 255)   // Magenta for TEC 3 Voltage (if needed)
    };

    
    if (!currentData_.empty()) {
        size_t tecCount = currentData_.front().size(); 
        for (size_t tec = 0; tec < tecCount; ++tec) {  
            std::string label = currentLabels_[tec];
            if (tecCount > checkboxes_.size()) { 
                return;
            }
            if (!checkboxes_[tec]->IsChecked()) {
                continue;  
            }
            dc.SetPen(wxPen(wxColour(255, 0, 0), 2));  
            for (size_t i = 1; i < currentData_.size(); ++i) {
                if (tec < currentData_[i - 1].size() && tec < currentData_[i].size()) { 
                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((currentData_[i - 1][tec] - currentMin_) * yScaleCurrent);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y2 = height - 50 - static_cast<int>((currentData_[i][tec] - currentMin_) * yScaleCurrent);
                    dc.DrawLine(x1, y1, x2, y2);
                }
            }
        }

    }

    if (!voltageData_.empty()) {
        size_t tecCount = voltageData_.front().size(); 
        for (size_t tec = 0; tec < tecCount; ++tec) {       
            std::string label = voltageLabels_[tec];     
            if (!checkboxes_[tec]->IsChecked()) {              
                continue; 
            }
            dc.SetPen(wxPen(wxColour(0, 255, 0), 2));  
            for (size_t i = 1; i < voltageData_.size(); ++i) {
                if (tec < voltageData_[i - 1].size() && tec < voltageData_[i].size()) { 
                    int x1 = static_cast<int>((i - 1) * xStep + 50);
                    int x2 = static_cast<int>(i * xStep + 50);
                    int y1 = height - 50 - static_cast<int>((voltageData_[i - 1][tec] - voltageMin_) * yScaleVoltage);
                    int y2 = height - 50 - static_cast<int>((voltageData_[i][tec] - voltageMin_) * yScaleVoltage);
                    dc.DrawLine(x1, y1, x2, y2);
                }
            }
        }
    }
}
void GraphPlotting::drawYAxisLabels(wxDC& dc, int width, int height, bool leftAxis, float maxValue, float minValue, const wxString& unit) {
    
    int xPos = leftAxis ? 25 : width - 30;  
    const float maxDisplayValue = 1000;  
    float displayMaxValue = std::min(maxDisplayValue, maxValue); 
    for (int i = 0; i <= 10; i++) {
        float value = minValue + i * (displayMaxValue - minValue) / 10; 
        wxString label = wxString::Format("%.2f %s", value, unit); 
        int yPos = height - 50 - (i * (height - 100) / 10); 
        dc.DrawText(label, wxPoint(xPos, yPos));
    }
}
void GraphPlotting::paintEvent(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);  
    render(dc); 
}
void GraphPlotting::OnResize(wxSizeEvent& event) {
    this->Refresh();
    event.Skip();
}

void GraphPlotting::RefreshGraph() {
    this->Refresh();
    this->Update();   
}