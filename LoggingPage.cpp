#include "CustomLogDebugOutput.h"
#include "..\LaserGUI\GraphPlotting.h"
#include "LoggingPage.h"
#include "../CommonFunctions_GUI.h"
#include "../LaserGUI/GraphingWork.h" 

using namespace std;


static wxString LOGGING_STR = _("Logging");
static wxString CUSTOM_LOGGING_STR = _("Custom Logging");
static wxString CUSTOM_LOGGING_TOOLTIP = _(
	"Record custom laser data to a comma-separated log file.\n"
	"\n"
	" 1. Select which laser data you want to record.\n"
	" 2. Select a location to save log data to a new or existing file.\n"
	" 3. Choose a time interval for each data point.\n"
	" 4. Click \"Start\".\n"
	"\n"
	"The GUI will now log data continuously until you click \"Stop\" or close the GUI.\n"
	"You may save a new, separate copy of the data to another file at any time by clicking \"Save Now\"."
);
static wxString SELECT_DATA_TO_RECORD_STR = _("Select Data to Record");
////////////////////////////////////////////////////////////////////////////////////////////
static wxString REAL_TIME_TEMP_LOG_STR = _("Real-Time Temperature Logs:");
////////////////////////////////////////////////////////////////////////////////////////////
static wxString SELECT_LOG_OUTPUT_FILE_STR = _("Select Log Output File");
static wxString SELECT_STR = _("Select");
static wxString TIME_INTERVAL_STR = _("Time Interval");
static wxString SECONDS_STR = _("seconds");
static wxString STOP_STR = _("Stop");
static wxString TOTAL_LOG_TIME_STR = _("Total Log Time:");
static wxString TOTAL_DATA_POINTS_STR = _("Total Data Points:");
static wxString RESET_STR = _("Reset");
static wxString SAVE_NOW_STR = _("Save Now");


const vector<LaserStateLogCategoryEnum> LASER_STATE_LOG_CATEGORIES_VISIBLE_TO_USER{
	POWER,
	DIODE_CURRENTS,
	TEMPERATURES,
	SENSORS,
	PULSE_INFO,
	MOTORS,
	ALARMS,
};


LoggingPage::LoggingPage(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) :
	SettingsPage_Base(_lc, parent) {



	logger = make_shared<CustomLogger>(lc);
	CustomLogDebugOutput* logDebugOutput = new CustomLogDebugOutput();
	logger->addObserver(logDebugOutput);


	logTimer.Bind(wxEVT_TIMER, &LoggingPage::OnLogTimer, this, logTimer.GetId());


	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	CustomLoggingPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	CustomLoggingPanel->SetFont(FONT_SMALL_SEMIBOLD);
	CustomLoggingPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* CustomLoggingSizer = new wxBoxSizer(wxVERTICAL);

	CustomLoggingTitle = new FeatureTitle(CustomLoggingPanel, CUSTOM_LOGGING_STR, _(CUSTOM_LOGGING_TOOLTIP));
	CustomLoggingSizer->Add(CustomLoggingTitle, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	wxGridBagSizer* CustomLoggingControlsSizer = new wxGridBagSizer(0, 0);

	SelectDataPanel = new wxPanel(CustomLoggingPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);

	wxBoxSizer* SelectDataSizer = new wxBoxSizer(wxVERTICAL);

	SelectDataLabel = new wxStaticText(SelectDataPanel, wxID_ANY, SELECT_DATA_TO_RECORD_STR, wxDefaultPosition, wxDefaultSize, 0);
	SelectDataLabel->SetFont(FONT_SMALL_SEMIBOLD);
	SelectDataSizer->Add(SelectDataLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	LogDataCheckboxesSizer = new wxBoxSizer(wxVERTICAL);



	SelectDataSizer->Add(LogDataCheckboxesSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);


	SelectDataPanel->SetSizer(SelectDataSizer);
	SelectDataPanel->Layout();
	SelectDataSizer->Fit(SelectDataPanel);
	CustomLoggingControlsSizer->Add(SelectDataPanel, wxGBPosition(0, 0), wxGBSpan(2, 1), wxALL | wxEXPAND, 5);

	// Select Log Output File
	LogOutputFilePanel = new wxPanel(CustomLoggingPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);

	wxBoxSizer* LogOutputFileSizer = new wxBoxSizer(wxVERTICAL);

	LogOutputFileLabel = new wxStaticText(LogOutputFilePanel, wxID_ANY, _(SELECT_LOG_OUTPUT_FILE_STR), wxDefaultPosition, wxDefaultSize, 0);
	LogOutputFileLabel->SetFont(FONT_SMALL_SEMIBOLD);
	LogOutputFileSizer->Add(LogOutputFileLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	LogOutputFileTextCtrl = new wxTextCtrl(LogOutputFilePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_READONLY | wxTE_BESTWRAP);
	LogOutputFileTextCtrl->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogOutputFileSizer->Add(LogOutputFileTextCtrl, 0, wxALL, 5);

	SelectLogOutputFileButton = new wxButton(LogOutputFilePanel, wxID_ANY, _(SELECT_STR), wxDefaultPosition, wxDefaultSize, 0);
	SelectLogOutputFileButton->Bind(wxEVT_BUTTON, &LoggingPage::OnSelectLogOutputFileButtonClicked, this);
	SelectLogOutputFileButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	LogOutputFileSizer->Add(SelectLogOutputFileButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	LogOutputFilePanel->SetSizer(LogOutputFileSizer);
	LogOutputFilePanel->Layout();
	LogOutputFileSizer->Fit(LogOutputFilePanel);
	CustomLoggingControlsSizer->Add(LogOutputFilePanel, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL, 5);


	LogControlsPanel = new wxPanel(CustomLoggingPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);

	wxBoxSizer* LogControlsSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* TimeIntervalSizer = new wxBoxSizer(wxHORIZONTAL);


	// Time Interval
	TimeIntervalLabel = new wxStaticText(LogControlsPanel, wxID_ANY, _(TIME_INTERVAL_STR), wxDefaultPosition, wxDefaultSize, 0);
	TimeIntervalLabel->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	TimeIntervalSizer->Add(TimeIntervalLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

	TimeIntervalTextCtrl = new NumericTextCtrl(LogControlsPanel, NumericTextCtrlType::DIGITS_ONLY, 6, wxSize(60, -1));
	TimeIntervalTextCtrl->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	TimeIntervalTextCtrl->SetHelpText(wxT("Choose the amount of time between each log event."));
	TimeIntervalSizer->Add(TimeIntervalTextCtrl, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);

	TimeIntervalUnits = new wxStaticText(LogControlsPanel, wxID_ANY, _(SECONDS_STR), wxDefaultPosition, wxDefaultSize, 0);
	TimeIntervalUnits->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	TimeIntervalSizer->Add(TimeIntervalUnits, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	LogControlsSizer->Add(TimeIntervalSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);


	// Start button
	StartButton = new wxButton(LogControlsPanel, wxID_ANY, _(START_TEXT), wxDefaultPosition, wxSize(100, 35), 0);
	StartButton->Bind(wxEVT_BUTTON, &LoggingPage::OnStartButtonClicked, this);
	StartButton->SetFont(FONT_MED_SMALL_SEMIBOLD);
	StartButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	LogControlsSizer->Add(StartButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	// Log status message
	LogStatusMessage = new DynamicStatusMessage(LogControlsPanel, wxEmptyString, -1, 4, 4);
	LogControlsSizer->Add(LogStatusMessage, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	wxFlexGridSizer* LogStatRowsSizer = new wxFlexGridSizer(0, 2, 0, 0);

	// Total Log Time
	TotalLogTimeLabel = new wxStaticText(LogControlsPanel, wxID_ANY, _(TOTAL_LOG_TIME_STR), wxDefaultPosition, wxDefaultSize, 0);
	TotalLogTimeLabel->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogStatRowsSizer->Add(TotalLogTimeLabel, 0, wxALL | wxALIGN_RIGHT, 5);

	TotalLogTimeValue = new wxStaticText(LogControlsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0);
	TotalLogTimeValue->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogStatRowsSizer->Add(TotalLogTimeValue, 0, wxALL, 5);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Real-time Log Temperature
	wxBoxSizer* RealTimeLogSizer = new wxBoxSizer(wxVERTICAL);

	// Creating and Adding the Static Text Label:
	wxStaticText* RealTimeTempLogLabel = new wxStaticText(LogControlsPanel, wxID_ANY, _("Real-Time Temperature Logs:"), wxDefaultPosition, wxDefaultSize, 0);
	RealTimeTempLogLabel->SetFont(FONT_SMALL_SEMIBOLD);
	RealTimeLogSizer->Add(RealTimeTempLogLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	// Creating and Adding the wxTextCtrl:
	RealTimeTempLogTextCtrl = new wxTextCtrl(LogControlsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE | wxTE_READONLY);
	RealTimeTempLogTextCtrl->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	RealTimeLogSizer->Add(RealTimeTempLogTextCtrl, 0, wxALL | wxEXPAND, 5);

	// Add the RealTimeLogSizer to LogControlsPanel's sizer
	LogControlsSizer->Add(RealTimeLogSizer, 0, wxALL | wxEXPAND, 5);

	// Creating and Adding the Graph (GraphPlotting)
	wxBoxSizer* plotSizer = new wxBoxSizer(wxVERTICAL);

	// Initialize the plot canvas
	//GraphPlotting* graphPlotting = new GraphPlotting(LogControlsPanel, wxID_ANY, wxDefaultPosition, wxSize(400, 200), checkboxes);


	// Add the plot canvas to the plot sizer
	//LogControlsSizer->Add(graphPlotting, 1, wxEXPAND | wxALL, 5);

	// Add the plotSizer to your existing layout
	LogControlsSizer->Add(plotSizer, 0, wxALL | wxEXPAND, 5);

	// Setting the LogControlsPanel sizer to the LoggingPage layout
	this->SetSizerAndFit(LogControlsPanel->GetSizer());

	// Ensure layout is updated
	LogControlsSizer->Layout();

	// Initialize RealTimeObserver with the wxTextCtrl and the plot canvas
	//RealTimeObserver* tempObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, graphPlotting);
	//logger->addObserver(tempObserver);


	// Bind the timer event
	logTimer.Bind(wxEVT_TIMER, &LoggingPage::OnLogTimer, this, logTimer.GetId());





	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Total Data Points
	TotalDataPointsLabel = new wxStaticText(LogControlsPanel, wxID_ANY, _(TOTAL_DATA_POINTS_STR), wxDefaultPosition, wxDefaultSize, 0);
	TotalDataPointsLabel->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogStatRowsSizer->Add(TotalDataPointsLabel, 0, wxALL | wxALIGN_RIGHT, 5);

	TotalDataPointsValue = new wxStaticText(LogControlsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0);
	TotalDataPointsValue->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogStatRowsSizer->Add(TotalDataPointsValue, 0, wxALL, 5);

	LogControlsSizer->Add(LogStatRowsSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);


	// Reset button
	ResetLogButton = new wxButton(LogControlsPanel, wxID_ANY, _(RESET_STR), wxDefaultPosition, wxDefaultSize, 0);
	ResetLogButton->Bind(wxEVT_BUTTON, &LoggingPage::OnResetButtonClicked, this);
	ResetLogButton->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogControlsSizer->Add(ResetLogButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	// Save log now button
	SaveLogNowButton = new wxButton(LogControlsPanel, wxID_ANY, _(SAVE_NOW_STR), wxDefaultPosition, wxDefaultSize, 0);
	SaveLogNowButton->Bind(wxEVT_BUTTON, &LoggingPage::OnSaveNowButtonClicked, this);
	SaveLogNowButton->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	LogControlsSizer->Add(SaveLogNowButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	LogControlsPanel->SetSizer(LogControlsSizer);
	LogControlsPanel->Layout();
	LogControlsSizer->Fit(LogControlsPanel);
	CustomLoggingControlsSizer->Add(LogControlsPanel, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, 5);

	CustomLoggingSizer->Add(CustomLoggingControlsSizer, 1, wxEXPAND, 5);

	CustomLoggingPanel->SetSizer(CustomLoggingSizer);
	CustomLoggingPanel->Layout();
	CustomLoggingSizer->Fit(CustomLoggingPanel);
	sizer->Add(CustomLoggingPanel, 0, wxALL, 5);

	Init();

	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

}


void LoggingPage::Init() {

	InitCategoryCheckboxes();
	TimeIntervalTextCtrl->SetLabelText(to_wx_string(logger->GetTimeIntervalInSeconds()));
	RefreshControlsEnabled();


}


void LoggingPage::InitCategoryCheckboxes() {
	categoryCheckboxes.clear();
	for (auto& categoryCheckbox : LogDataCheckboxesSizer->GetChildren())
		categoryCheckbox->DeleteWindows();
	LogDataCheckboxesSizer->Clear();
	Layout();
	Refresh();

	vector<LaserStateLogCategoryEnum> categoriesToInclude;
	if (GetGUIAccessMode() == GuiAccessMode::END_USER)
		categoriesToInclude = LASER_STATE_LOG_CATEGORIES_VISIBLE_TO_USER;
	else
		categoriesToInclude = LASER_STATE_LOG_CATEGORIES;

	for (LaserStateLogCategoryEnum category : categoriesToInclude) {
		LogCategoryCheckbox* checkbox = new LogCategoryCheckbox(lc, SelectDataPanel, logger, category);
		categoryCheckboxes.push_back(checkbox);
		LogDataCheckboxesSizer->Add(checkbox, 0, wxALL, 3);
	}
	Layout();
	Refresh();
}


void LoggingPage::OnSelectLogOutputFileButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Select Log Output File button clicked")
		wxString defaultLogFileName = "LaserStateLog_(" + lc->GetLaserModel() + ")_(Serial#" + lc->GetSerialNumber() + ")_(" + GenerateDateString() + ")";

	wxFileDialog selectOutputFileDialog(this, (_(SELECT_LOG_OUTPUT_FILE_STR)), "", defaultLogFileName, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (selectOutputFileDialog.ShowModal() == wxID_CANCEL)
		return;

	string path = string(selectOutputFileDialog.GetPath());
	STAGE_ACTION_ARGUMENTS(path)
		if (PathIsValid(path)) {
			logger->SetFilePath(path);
			logger->Reset();
			totalLogTimeInS = 0;
			LogOutputFileTextCtrl->SetLabelText(path);
			LogStatusMessage->SetLabelText("");
		}
	RefreshControlsEnabled();
	LOG_ACTION()
}



void LoggingPage::OnStartButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Start logging button clicked")

		if (logger->IsLogging()) {
			// Stop logging if already in progress
			STAGE_ACTION_ARGUMENTS("Stop")
				logger->Stop();
			logTimer.Stop();
			LogStatusMessage->StopCycling();
			LogStatusMessage->Set(_("Paused"));
		}
		else {
			// Start logging
			STAGE_ACTION_ARGUMENTS("Start")
				logger->SetTimeIntervalInSeconds(stoi(string(TimeIntervalTextCtrl->GetValue())));
			logger->Start();
			logTimer.Start(1000);
			LogStatusMessage->StartCycling();
			LogStatusMessage->Set(_("Logging"));

			// Create a new graph window
			wxFrame* graphWindow = new wxFrame(this, wxID_ANY, _("Graph Window"), wxDefaultPosition, wxSize(1200, 600));
			wxPanel* panel = new wxPanel(graphWindow, wxID_ANY);

			std::vector<wxCheckBox*> checkboxes;


			// Now create checkboxes for the TECs and bind them to their respective plots
			wxBoxSizer* checkboxSizer = new wxBoxSizer(wxHORIZONTAL);
			vector<int> tecIDs = lc->GetTemperatureControlIDs();
			GraphPlotting* currentPlot = nullptr;
			GraphPlotting* voltagePlot = nullptr;
			GraphPlotting* tempPlot = nullptr;


			for (int id : tecIDs) {
				std::string label = lc->GetTemperatureControlLabel(id);

				
				wxCheckBox* checkBox = new wxCheckBox(panel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);

				checkBox->Bind(wxEVT_CHECKBOX, [&, currentPlot,voltagePlot,tempPlot](wxCommandEvent& event) {
					if (currentPlot) { 
						currentPlot->RefreshGraph(); 
					}
					else if (voltagePlot) {  
						voltagePlot->RefreshGraph();  
					}
					else if (tempPlot) {  
						tempPlot->RefreshGraph();  
					}

					});
				
				checkboxes_.push_back(checkBox);
				checkboxSizer->Add(checkBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}

			 currentPlot = new GraphPlotting(panel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), checkboxes_);
			currentPlot->SetMinSize(wxSize(600, 150));

			 voltagePlot = new GraphPlotting(panel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), checkboxes_);
			voltagePlot->SetMinSize(wxSize(600, 150));

			 tempPlot = new GraphPlotting(panel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), checkboxes_);
			tempPlot->SetMinSize(wxSize(600, 150));

			
			RealTimeObserver* tempObserverForWindow = new RealTimeObserver(RealTimeTempLogTextCtrl, currentPlot, voltagePlot, tempPlot);
			logger->addObserver(tempObserverForWindow);



			wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
			// Add the checkboxSizer to the main sizer
			mainSizer->Add(checkboxSizer, 0, wxEXPAND | wxALL, 5);
			mainSizer->Add(currentPlot, 1, wxEXPAND | wxALL, 5);
			mainSizer->Add(voltagePlot, 1, wxEXPAND | wxALL, 5);
			mainSizer->Add(tempPlot, 1, wxEXPAND | wxALL, 5);

			panel->SetSizer(mainSizer);

		
			graphWindow->Show();

		
			RefreshControlsEnabled();
			LOG_ACTION()
		}
}

/*
			// Create the graphing window using the new class
			GraphingWork graphingWork(this, logger, RealTimeTempLogTextCtrl, lc);
			graphingWork.CreateGraphWindow();  // Create the graph window and setup the graph*/


void LoggingPage::OnResetButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Reset log button clicked")
		logger->Reset();
	totalLogTimeInS = 0;
	if (!logger->IsLogging())
		LogStatusMessage->Set(_("Log reset"));
	RefreshAll();
	RefreshControlsEnabled();
	LOG_ACTION()
}


void LoggingPage::OnSaveNowButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Save log now button clicked")
		wxString defaultLogFileName = "LaserStateLog_(" + lc->GetLaserModel() + ")_(Serial#" + lc->GetSerialNumber() + ")_(" + GenerateDateString() + ")";

	wxFileDialog selectOutputFileDialog(this, (_(SELECT_LOG_OUTPUT_FILE_STR)), "", defaultLogFileName, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (selectOutputFileDialog.ShowModal() == wxID_CANCEL)
		return;

	string path = string(selectOutputFileDialog.GetPath());
	STAGE_ACTION_ARGUMENTS(path)
		if (PathIsValid(path)) {
			logger->SaveMemoryLogToFile(path);
			if (!logger->SaveSuccessful())
				wxMessageBox("Failed to save log file. You may not have permission to save there.");
		}
	RefreshControlsEnabled();
	LOG_ACTION()
}


void LoggingPage::OnLogTimer(wxTimerEvent& event) {
	totalLogTimeInS++;
	//wxLogMessage("Log Timer triggered");
	// Your timer handling code here
}


// Refresh whether widgets are enabled or disabled based on whether logger is
// logging and whether it has already logged data points. Only called after
// certain actions to prevent constant flickering.
void LoggingPage::RefreshControlsEnabled() {
	bool isLogging = logger->IsLogging();
	bool hasLoggedDataPoints = logger->GetTotalLoggedDataPoints() > 0;

	// Category Checkboxes
	for (auto checkbox : categoryCheckboxes)
		checkbox->RefreshEnableStatus();

	// Select Log Output
	RefreshWidgetEnableBasedOnCondition(SelectLogOutputFileButton, !isLogging);

	// Start Button
	SetBGColorBasedOnCondition(StartButton, isLogging, TEXT_COLOR_LIGHT_GREEN, BUTTON_COLOR_INACTIVE);
	SetTextBasedOnCondition(StartButton, isLogging, _(STOP_STR), _(START_TEXT));
	SetTextBasedOnCondition(StartButton, isLogging, _(STOP_STR), _(START_TEXT));
	RefreshWidgetEnableBasedOnCondition(StartButton, logger->SetFilePathSuccessful());

	// Log Status Message
	if (isLogging)
		LogStatusMessage->StartCycling();

}


void LoggingPage::RefreshAll() {
	TotalLogTimeValue->SetLabelText(to_wx_string(totalLogTimeInS) + " s");
	TotalDataPointsValue->SetLabelText(to_wx_string(logger->GetTotalLoggedDataPoints()));

	bool hasLoggedDataPoints = logger->GetTotalLoggedDataPoints() > 0;
	RefreshWidgetEnableBasedOnCondition(TimeIntervalTextCtrl, !logger->IsLogging() and !hasLoggedDataPoints);
	RefreshWidgetEnableBasedOnCondition(ResetLogButton, hasLoggedDataPoints);
	RefreshWidgetEnableBasedOnCondition(SaveLogNowButton, hasLoggedDataPoints);
}


void LoggingPage::RefreshVisibility() {
	InitCategoryCheckboxes();
}


void LoggingPage::RefreshStrings() {
	SetName(_(LOGGING_STR));

	CustomLoggingTitle->RefreshStrings();
	SelectDataLabel->SetLabelText(_(SELECT_DATA_TO_RECORD_STR));
	LogOutputFileLabel->SetLabelText(_(SELECT_LOG_OUTPUT_FILE_STR));
	SelectLogOutputFileButton->SetLabelText(_(SELECT_STR));
	TimeIntervalLabel->SetLabelText(_(TIME_INTERVAL_STR));
	TimeIntervalUnits->SetLabelText(_(SECONDS_STR));
	TotalLogTimeLabel->SetLabelText(_(TOTAL_LOG_TIME_STR));
	TotalDataPointsLabel->SetLabelText(_(TOTAL_DATA_POINTS_STR));
	ResetLogButton->SetLabelText(_(RESET_STR));
	SaveLogNowButton->SetLabelText(_(SAVE_NOW_STR));
	for (auto checkbox : categoryCheckboxes)
		checkbox->RefreshStrings();
	this->Layout();
	this->Refresh();
}



// LogCategoryCheckbox implementation

LogCategoryCheckbox::LogCategoryCheckbox(shared_ptr<MainLaserControllerInterface> laser_controller, wxWindow* parent, shared_ptr<CustomLogger> _logger, LaserStateLogCategoryEnum _category) :
	wxCheckBox(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0) {
	lc = laser_controller;
	logger = _logger;
	category = _category;
	categoryName = logger->GetCategoryName(category);

	this->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	this->Bind(wxEVT_CHECKBOX, &LogCategoryCheckbox::OnLogDataCategoryCheckboxChecked, this);
	RefreshStrings();
}

void LogCategoryCheckbox::OnLogDataCategoryCheckboxChecked(wxCommandEvent& evt) {
	STAGE_ACTION("Log data category checkbox clicked")
		if (this->IsChecked()) {
			logger->IncludeCategory(category);
			STAGE_ACTION_ARGUMENTS("Included " + categoryName)
		}
		else {
			logger->ExcludeCategory(category);
			STAGE_ACTION_ARGUMENTS("Excluded " + categoryName)
		}
	LOG_ACTION()
}

void LogCategoryCheckbox::RefreshEnableStatus() {
	// Can only include/exclude log data categories when not logging and no data points have been logged yet.
	bool canChangeCategoriesIncluded = !logger->IsLogging() and logger->GetTotalLoggedDataPoints() == 0;
	RefreshWidgetEnableBasedOnCondition(this, canChangeCategoriesIncluded);
}

void LogCategoryCheckbox::RefreshStrings() {
	this->SetLabelText(_(categoryName));
}
