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

// Updated to handle vectors for multiple TECs' current and voltage
void GraphWindow::UpdateGraph(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time) {
    // Pass vectors of current and voltage along with time to GraphPlotting
    graphPlotting_->AddDataPoint(currents, voltages, time);
    graphPlotting_->RefreshGraph();  // Refresh graph rendering
}
