#pragma once

#include <map>
#include "wx/wx.h"
#include <wx/overlay.h>

#include "SettingsPage_Base.h"
#include "Loggers/CustomLogger.h"
#include "../CommonGUIComponents/FeatureTitle.h"
#include "../CommonGUIComponents/DynamicStatusMessage.h"
#include "../CommonGUIComponents/NumericTextCtrl.h"
#include "../LaserGUI/RealTimeObserver.h"
#include "../LaserGUI/SettingsPages/AutotunePlotCanvas.h"
//#include "../LaserGUI/GraphingWork.h"


class LogCategoryCheckbox : public wxCheckBox {

private:
	std::shared_ptr<MainLaserControllerInterface> lc = nullptr;
	std::shared_ptr<CustomLogger> logger = nullptr;
	LaserStateLogCategoryEnum category = NULL_CATEGORY;
	string categoryName;

	void OnLogDataCategoryCheckboxChecked(wxCommandEvent& evt);
	


public:
	LogCategoryCheckbox(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent, shared_ptr<CustomLogger> _logger, LaserStateLogCategoryEnum _category);
	void RefreshEnableStatus();
	void RefreshStrings();
};


class LoggingPage : public SettingsPage_Base {

public:
	LoggingPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);



	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void OnStartButtonClicked(wxCommandEvent& evt);
	void OnCheckboxToggle(wxCommandEvent& event);




private:

	std::shared_ptr<CustomLogger> logger;
	std::vector<LogCategoryCheckbox*> categoryCheckboxes;
	wxTimer logTimer;
	std::function<void()> updateLayout__;
	

	unsigned int totalLogTimeInS = 0;
	GraphPlotting* graphPlot_;
	wxPanel* CustomLoggingPanel;
	FeatureTitle* CustomLoggingTitle;
	wxPanel* SelectDataPanel;
	wxStaticText* SelectDataLabel;
	wxBoxSizer* LogDataCheckboxesSizer;
	wxCheckBox* LogDataCheckbox_Alarms;
	wxCheckBox* LogDataCheckbox_DiodeCurrents;
	wxPanel* LogOutputFilePanel;
	wxStaticText* LogOutputFileLabel;
	wxTextCtrl* LogOutputFileTextCtrl;
	wxButton* SelectLogOutputFileButton;
	wxPanel* LogControlsPanel;
	wxStaticText* TimeIntervalLabel;
	NumericTextCtrl* TimeIntervalTextCtrl;
	wxStaticText* TimeIntervalUnits;
	wxButton* StartButton;
	DynamicStatusMessage* LogStatusMessage;
	wxStaticText* TotalLogTimeLabel;
	wxStaticText* TotalLogTimeValue;
	wxStaticText* TotalDataPointsLabel;
	wxStaticText* TotalDataPointsValue;
	wxButton* ResetLogButton;
	wxButton* SaveLogNowButton;
	wxCheckBox* checkboxes;

	wxCheckBox* alarmCheckbox;        // Checkbox for alarms
	wxCheckBox* tecCurrentCheckbox;  // Checkbox for TEC Current
	wxCheckBox* diodeCheckbox;       // Checkbox for Diode Current
	wxCheckBox* powerCheckbox;       // Checkbox for Power Monitor
	wxCheckBox* sensorCheckbox;      // Checkbox for Sensors

	wxPanel* alarmPanel;             // Alarm panel reference
	wxPanel* tecPanel;               // TEC panel reference
	wxPanel* diodePanel;             // Diode panel reference
	wxPanel* powerPanel;             // Power panel reference
	wxPanel* sensorPanel;            // Sensor panel reference

	wxBoxSizer* mainSizer;           // Main sizer for layout adjustments
	wxPanel* powerContentPanel;
	wxBitmapButton* powerToggleButton;
	 

private:
	wxTextCtrl* RealTimeTempLogTextCtrl;
	RealTimeObserver* tempObserver;

	std::vector<wxCheckBox*> checkboxes_;

	void InitCategoryCheckboxes();

	void RefreshControlsEnabled();
	void CreateChartPanel();

	void OnSelectLogOutputFileButtonClicked(wxCommandEvent& evt);

	void OnResetButtonClicked(wxCommandEvent& evt);
	void OnSaveNowButtonClicked(wxCommandEvent& evt);
	void OnLogTimer(wxTimerEvent& evt);


	

};





