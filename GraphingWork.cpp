#include "GraphingWork.h"
#include "wx/wx.h"

GraphingWork::GraphingWork(wxWindow* parent, std::shared_ptr<CustomLogger> logger, wxTextCtrl* realTimeLogTextCtrl, std::shared_ptr<MainLaserControllerInterface> lc)
    : parent_(parent), logger_(logger), realTimeLogTextCtrl_(realTimeLogTextCtrl), lc_(lc), graphPlot_(nullptr), graphWindow_(nullptr) {
}

void GraphingWork::CreateGraphWindow() {
    // Create the graph window
    graphWindow_ = new wxFrame(parent_, wxID_ANY, _("Graph Window"), wxDefaultPosition, wxSize(1200, 600));
    SetupGraph();  // Set up the graph inside the window
    graphWindow_->Show();  // Show the graph window
}

void GraphingWork::SetupGraph() {
    wxPanel* panel = new wxPanel(graphWindow_, wxID_ANY);
   
    std::vector<wxCheckBox*> checkboxes;
    wxBoxSizer* checkboxSizer = new wxBoxSizer(wxHORIZONTAL);

    // Fetch TEC IDs and create checkboxes dynamically for each TEC
    vector<int> tecIDs = lc_->GetTemperatureControlIDs();
    GraphPlotting* graphPlot = nullptr;
    for (int id : tecIDs) {
        std::string label = lc_->GetTemperatureControlLabel(id);
        wxCheckBox* checkBox = new wxCheckBox(panel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
       // wxLogMessage("Creating checkbox for TEC: %s", label);  // Log creation of each checkbox

        // After initializing graphPlot_ in SetupGraph()
        for (auto& checkBox : checkboxes) {
            checkBox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
                if (graphPlot_) {
                    graphPlot_->RefreshGraph();
                }
                });
        }

        checkboxes.push_back(checkBox);
        checkboxSizer->Add(checkBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    }
    graphPlot_ = new GraphPlotting(panel, wxID_ANY, wxDefaultPosition, wxSize(1000, 500), checkboxes);
    graphPlot_->SetMinSize(wxSize(1100, 700));

    // Continue with observer and layout setup
    RealTimeObserver* tempObserverForWindow = new RealTimeObserver(realTimeLogTextCtrl_, graphPlot_);
    logger_->addObserver(tempObserverForWindow);

    // Set up layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(checkboxSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(graphPlot_, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
}

