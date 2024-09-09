#pragma once

#include "wx/wx.h"
#include "GraphPlotting.h"  // Include the full GraphPlotting class

class GraphWindow : public wxFrame {
public:
    GraphWindow(wxWindow* parent, const wxString& title, const wxSize& size);
    void UpdateGraph(float tec1Current, float tec2Current, float tec1Voltage, float tec2Voltage, const wxString& time);

private:
    GraphPlotting* graphPlotting_;  // GraphPlotting object for rendering the graph
};
