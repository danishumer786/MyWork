#include "GraphWindow.h"
#include "GraphPlotting.h"  // Include the full GraphPlotting class

GraphWindow::GraphWindow(wxWindow* parent, const wxString& title, const wxSize& size)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, size) {

    // Create GraphPlotting panel
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    graphPlotting_ = new GraphPlotting(this, wxID_ANY, wxDefaultPosition, size);
    sizer->Add(graphPlotting_, 1, wxEXPAND | wxALL, 5);

    this->SetSizerAndFit(sizer);
    this->Show(true);
}

// Updated to handle four data points: TEC1 Current, TEC2 Current, TEC1 Voltage, TEC2 Voltage
void GraphWindow::UpdateGraph(float tec1Current, float tec2Current, float tec1Voltage, float tec2Voltage, const wxString& time) {
    // Pass four values (two for current, two for voltage) along with time to GraphPlotting
    graphPlotting_->AddDataPoint(tec1Current, tec2Current, tec1Voltage, tec2Voltage, time);
    graphPlotting_->RefreshGraph();  // Refresh graph rendering
}
