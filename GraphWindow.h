#pragma once

#include "wx/wx.h"
#include "GraphPlotting.h"  // Include the full GraphPlotting class

class GraphWindow : public wxFrame {
public:
    GraphWindow(wxWindow* parent, const wxString& title, const wxSize& size);

    // Modify this function to accept vectors of currents and voltages
    void UpdateGraph(const std::vector<float>& currents, const std::vector<float>& voltages, const wxString& time);

private:
    GraphPlotting* graphPlotting_;  // GraphPlotting object for rendering the graph
};
