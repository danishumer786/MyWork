#pragma once

#include "wx/wx.h"
#include "GraphPlotting.h"
#include "RealTimeObserver.h"
#include <vector>
#include "../CommonUtilities/Logging/LogObserver.h"
#include "Loggers/CustomLogger.h"

class GraphingWork {
public:
    GraphingWork(wxWindow* parent, std::shared_ptr<CustomLogger> logger, wxTextCtrl* realTimeLogTextCtrl, std::shared_ptr<MainLaserControllerInterface> lc);

    void CreateGraphWindow();  // Function to create the graph window and set up the plotting logic

private:
    wxWindow* parent_;
    std::shared_ptr<CustomLogger> logger_;
    wxTextCtrl* realTimeLogTextCtrl_;
    std::shared_ptr<MainLaserControllerInterface> lc_;
    std::vector<wxCheckBox*> checkboxes_;


    wxFrame* graphWindow_;
    GraphPlotting* graphPlot_;

    void SetupGraph();  // Internal function to create checkboxes and initialize graphPlot
};