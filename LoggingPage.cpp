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
static wxString REAL_TIME_TEMP_LOG_STR = _("Real-Time Selected Category Logs:");
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


LoggingPage::LoggingPage(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) :SettingsPage_Base(_lc, parent) {
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



	// Real-time TextCrl to show up data of all selected Categories
	wxBoxSizer* RealTimeLogSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* RealTimeTempLogLabel = new wxStaticText(LogControlsPanel, wxID_ANY, _("Real-Time Temperature Logs:"), wxDefaultPosition, wxDefaultSize, 0);
	RealTimeTempLogLabel->SetFont(FONT_SMALL_SEMIBOLD);
	RealTimeLogSizer->Add(RealTimeTempLogLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	RealTimeTempLogTextCtrl = new wxTextCtrl(LogControlsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE | wxTE_READONLY);
	RealTimeTempLogTextCtrl->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	RealTimeLogSizer->Add(RealTimeTempLogTextCtrl, 0, wxALL | wxEXPAND, 5);
	LogControlsSizer->Add(RealTimeLogSizer, 0, wxALL | wxEXPAND, 5);
	wxBoxSizer* plotSizer = new wxBoxSizer(wxVERTICAL);
	LogControlsSizer->Add(plotSizer, 0, wxALL | wxEXPAND, 5);
	this->SetSizerAndFit(LogControlsPanel->GetSizer());
	LogControlsSizer->Layout();
	logTimer.Bind(wxEVT_TIMER, &LoggingPage::OnLogTimer, this, logTimer.GetId());




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
			STAGE_ACTION_ARGUMENTS("Stop")
				logger->Stop();
			logTimer.Stop();
			LogStatusMessage->StopCycling();
			LogStatusMessage->Set(_("Paused"));
		}
		else {
			STAGE_ACTION_ARGUMENTS("Start")
				logger->SetTimeIntervalInSeconds(stoi(string(TimeIntervalTextCtrl->GetValue())));
			logger->Start();
			logTimer.Start(1000);
			LogStatusMessage->StartCycling();
			LogStatusMessage->Set(_("Logging"));

			// Create a new graph window
			wxFrame* graphWindow = new wxFrame(this, wxID_ANY, _("Graph Window"), wxDefaultPosition, wxSize(1200, 600));

			wxPanel* mainPanel = new wxPanel(graphWindow, wxID_ANY);
			wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
			// --------------------------------------------------------------------------------------------------------------------------------------------------------

			// ******** Alarm Panel with Dynamic Toggle ********
			wxPanel* alarmPanel = new wxPanel(mainPanel, wxID_ANY);
			wxBoxSizer* alarmSizer = new wxBoxSizer(wxVERTICAL);

			wxBoxSizer* toggleSizer = new wxBoxSizer(wxHORIZONTAL);

			wxBitmap expandBitmap(wxT("Images/up_caret_small.png"), wxBITMAP_TYPE_PNG);
			wxBitmap collapseBitmap(wxT("Images/down_caret_small.png"), wxBITMAP_TYPE_PNG);

			wxBitmapButton* toggleButton = new wxBitmapButton(alarmPanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));

			wxStaticText* alarmLabel = new wxStaticText(alarmPanel, wxID_ANY, _("Alarms"), wxDefaultPosition, wxDefaultSize);

			toggleSizer->Add(toggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			toggleSizer->Add(alarmLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

			wxTextCtrl* alarmTextCtrl = new wxTextCtrl(alarmPanel, wxID_ANY, _("No alarms"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
			alarmTextCtrl->Hide(); // Initially hide the alarm text control

			alarmSizer->Add(toggleSizer, 0, wxEXPAND | wxALL, 5);
			alarmSizer->Add(alarmTextCtrl, 1, wxEXPAND | wxALL, 5);

			alarmPanel->SetSizer(alarmSizer);
			mainSizer->Add(alarmPanel, 0, wxEXPAND | wxALL, 5);

			// Toggle button to manually open/close the alarm panel
			toggleButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
				if (alarmTextCtrl->IsShown()) {
					alarmTextCtrl->Hide();
					toggleButton->SetBitmap(expandBitmap);
				}
				else {
					alarmTextCtrl->Show();
					toggleButton->SetBitmap(collapseBitmap);
				}
				alarmPanel->GetParent()->Layout();
				});

			// Observer to monitor real-time data and handle alarm updates
			// Observer to monitor real-time data and handle alarm updates
			RealTimeObserver* observer = new RealTimeObserver(RealTimeTempLogTextCtrl, alarmTextCtrl);
			logger->addObserver(observer);





			wxScrolledWindow* scrolledWindow = new wxScrolledWindow(mainPanel, wxID_ANY);
			scrolledWindow->SetScrollRate(10, 10);
			wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);

			// ******** TEC Panel with Proper Dynamic Adjustment and Initially Closed ********
			wxPanel* tecPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* tecSizer = new wxBoxSizer(wxVERTICAL);

			GraphPlotting* currentPlot = nullptr;
			GraphPlotting* voltagePlot = nullptr;
			GraphPlotting* tempPlot = nullptr;

			// ******** TEC Panel Toggle ********
			wxBoxSizer* tecPanelHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
			wxBitmapButton* tecPanelToggleButton = new wxBitmapButton(tecPanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			wxStaticText* tecPanelLabel = new wxStaticText(tecPanel, wxID_ANY, _("TEC Panel"), wxDefaultPosition, wxDefaultSize);
			tecPanelLabel->SetFont(tecPanelLabel->GetFont().Bold());
			tecPanelHeaderSizer->Add(tecPanelToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			tecPanelHeaderSizer->Add(tecPanelLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			tecSizer->Add(tecPanelHeaderSizer, 0, wxEXPAND | wxALL, 5);

			// ******** Main TEC Toggle Panel ********
			wxPanel* mainTecTogglePanel = new wxPanel(tecPanel, wxID_ANY);
			wxBoxSizer* mainTecToggleSizer = new wxBoxSizer(wxVERTICAL);

			// TEC Checkboxes for current, voltage, and temperature
			wxBoxSizer* tecCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);
			std::vector<wxCheckBox*> tecCheckboxes;

			vector<int> tecIDs = lc->GetTemperatureControlIDs();
			for (int id : tecIDs) {
				std::string label = lc->GetTemperatureControlLabel(id);
				wxCheckBox* tecCheckBox = new wxCheckBox(mainTecTogglePanel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
				tecCheckBox->SetValue(true);

				tecCheckBox->Bind(wxEVT_CHECKBOX, [&, currentPlot, voltagePlot, tempPlot](wxCommandEvent& event) {
					if (currentPlot) { currentPlot->RefreshGraph(); }
					if (voltagePlot) { voltagePlot->RefreshGraph(); }
					if (tempPlot) { tempPlot->RefreshGraph(); }
					});

				tecCheckboxes.push_back(tecCheckBox);
				tecCheckboxSizer->Add(tecCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}
			mainTecToggleSizer->Add(tecCheckboxSizer, 0, wxEXPAND | wxALL, 5);
			mainTecTogglePanel->SetSizer(mainTecToggleSizer);
			tecSizer->Add(mainTecTogglePanel, 0, wxEXPAND | wxALL, 5);

			// ******** Container for TEC Subsections ********
			wxPanel* tecContainer = new wxPanel(tecPanel, wxID_ANY);
			wxBoxSizer* tecContainerSizer = new wxBoxSizer(wxVERTICAL);
			tecContainer->SetSizer(tecContainerSizer);
			tecSizer->Add(tecContainer, 0, wxEXPAND | wxALL, 5);

			// Helper function to update layout dynamically
			auto updateLayout = [tecPanel, scrolledWindow]() {
				scrolledWindow->Layout();
				scrolledWindow->FitInside();
				tecPanel->Layout();
				};

			// Initially hide TEC Panel contents
			mainTecTogglePanel->Hide();
			tecContainer->Hide();
			tecPanelToggleButton->SetBitmap(expandBitmap);

			// Bind event for the TEC panel toggle button
			tecPanelToggleButton->Bind(wxEVT_BUTTON, [mainTecTogglePanel, tecContainer, tecPanelToggleButton, updateLayout, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = mainTecTogglePanel->IsShown();
				mainTecTogglePanel->Show(!isVisible);
				tecContainer->Show(!isVisible);
				tecPanelToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);
				updateLayout();
				});

			wxPanel* currentPanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* currentSizer = new wxBoxSizer(wxVERTICAL);
			currentPlot = new GraphPlotting(currentPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			currentPlot->SetMinSize(wxSize(1000, 300));
			currentSizer->Add(currentPlot, 1, wxEXPAND | wxALL, 5);
			currentPanel->SetSizer(currentSizer);;

			wxPanel* currentTogglePanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* currentToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* currentToggleLabel = new wxStaticText(currentTogglePanel, wxID_ANY, _("TEC Current"), wxDefaultPosition, wxDefaultSize);
			currentToggleLabel->SetFont(currentToggleLabel->GetFont().Bold());
			wxBitmapButton* currentToggleButton = new wxBitmapButton(currentTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			currentToggleSizer->Add(currentToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			currentToggleSizer->Add(currentToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			currentTogglePanel->SetSizer(currentToggleSizer);

			tecContainerSizer->Add(currentTogglePanel, 0, wxEXPAND | wxALL, 5);
			tecContainerSizer->Add(currentPanel, 0, wxEXPAND | wxALL, 5);
			currentPanel->Hide();

			currentToggleButton->Bind(wxEVT_BUTTON, [currentPanel, currentToggleButton, updateLayout, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = currentPanel->IsShown();
				currentPanel->Show(!isVisible);
				currentToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);
				updateLayout();
				});

			// ******** TEC Voltage Section ********
			wxPanel* voltagePanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* voltageSizer = new wxBoxSizer(wxVERTICAL);
			voltagePlot = new GraphPlotting(voltagePanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			voltagePlot->SetMinSize(wxSize(1000, 300));
			voltageSizer->Add(voltagePlot, 1, wxEXPAND | wxALL, 5);
			voltagePanel->SetSizer(voltageSizer);

			wxPanel* voltageTogglePanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* voltageToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* voltageToggleLabel = new wxStaticText(voltageTogglePanel, wxID_ANY, _("TEC Voltage"), wxDefaultPosition, wxDefaultSize);
			voltageToggleLabel->SetFont(voltageToggleLabel->GetFont().Bold());
			wxBitmapButton* voltageToggleButton = new wxBitmapButton(voltageTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			voltageToggleSizer->Add(voltageToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			voltageToggleSizer->Add(voltageToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			voltageTogglePanel->SetSizer(voltageToggleSizer);

			tecContainerSizer->Add(voltageTogglePanel, 0, wxEXPAND | wxALL, 5);
			tecContainerSizer->Add(voltagePanel, 0, wxEXPAND | wxALL, 5);
			voltagePanel->Hide();

			voltageToggleButton->Bind(wxEVT_BUTTON, [voltagePanel, voltageToggleButton, updateLayout, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = voltagePanel->IsShown();
				voltagePanel->Show(!isVisible);
				voltageToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);
				updateLayout();
				});

			// ******** TEC Temperature Section ********
			wxPanel* tempPanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* tempSizer = new wxBoxSizer(wxVERTICAL);
			tempPlot = new GraphPlotting(tempPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			tempPlot->SetMinSize(wxSize(1000, 300));
			tempSizer->Add(tempPlot, 1, wxEXPAND | wxALL, 5);
			tempPanel->SetSizer(tempSizer);

			wxPanel* tempTogglePanel = new wxPanel(tecContainer, wxID_ANY);
			wxBoxSizer* tempToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* tempToggleLabel = new wxStaticText(tempTogglePanel, wxID_ANY, _("TEC Temperature"), wxDefaultPosition, wxDefaultSize);
			tempToggleLabel->SetFont(tempToggleLabel->GetFont().Bold());
			wxBitmapButton* tempToggleButton = new wxBitmapButton(tempTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			tempToggleSizer->Add(tempToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			tempToggleSizer->Add(tempToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			tempTogglePanel->SetSizer(tempToggleSizer);

			tecContainerSizer->Add(tempTogglePanel, 0, wxEXPAND | wxALL, 5);
			tecContainerSizer->Add(tempPanel, 0, wxEXPAND | wxALL, 5);
			tempPanel->Hide();

			tempToggleButton->Bind(wxEVT_BUTTON, [tempPanel, tempToggleButton, updateLayout, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = tempPanel->IsShown();
				tempPanel->Show(!isVisible);
				tempToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);
				updateLayout();
				});




			/*RealTimeObserver* tecObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, currentPlot, voltagePlot, tempPlot);
			logger->addObserver(tecObserver);*/


			// RealTimeObserver setup for TEC Current
			RealTimeObserver* tecObserver = new RealTimeObserver(
				RealTimeTempLogTextCtrl,          // Log text control
				currentPlot,  
				voltagePlot,
				tempPlot,                         // GraphPlotting instance
				currentPanel,                     // TEC Current panel
				currentToggleButton,   
				voltagePanel,
				voltageToggleButton,
				tempPanel,
				tempToggleButton,                 // TEC Current toggle button
				tecContainer,                     // TEC Container panel
				mainTecTogglePanel,               // Main TEC toggle panel
				tecPanelToggleButton,             // TEC Panel toggle button
				expandBitmap,                     // Bitmap for "expand"
				collapseBitmap,                   // Bitmap for "collapse"
				updateLayout                      // Layout updater function...
			);

			logger->addObserver(tecObserver);

			// Finalize the TEC Panel
			tecPanel->SetSizer(tecSizer);
			scrollSizer->Add(tecPanel, 0, wxEXPAND | wxALL, 5);


			scrolledWindow->SetSizer(scrollSizer);
			scrolledWindow->FitInside();
			scrolledWindow->SetScrollRate(10, 10);





			// ******** Diode Panel with Proper Shrinking and Custom Toggle Images ********
			wxPanel* diodePanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* diodeSizer = new wxBoxSizer(wxVERTICAL);

			GraphPlotting* diodePlot = nullptr;

			auto diodeUpdateLayout = [scrolledWindow, diodePanel]() {
				scrolledWindow->Layout();
				scrolledWindow->FitInside();
				diodePanel->Layout();
				};


			wxPanel* diodeTogglePanel = new wxPanel(diodePanel, wxID_ANY);
			wxBoxSizer* diodeToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* diodeToggleLabel = new wxStaticText(diodeTogglePanel, wxID_ANY, _("Diode Current"), wxDefaultPosition, wxDefaultSize);
			diodeToggleLabel->SetFont(diodeToggleLabel->GetFont().Bold());
			wxBitmapButton* diodeToggleButton = new wxBitmapButton(diodeTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			diodeToggleSizer->Add(diodeToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			diodeToggleSizer->Add(diodeToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			diodeTogglePanel->SetSizer(diodeToggleSizer);
			diodeSizer->Add(diodeTogglePanel, 0, wxEXPAND | wxALL, 5);

			// Diode Panel Content
			wxPanel* diodeContentPanel = new wxPanel(diodePanel, wxID_ANY);
			wxBoxSizer* diodeContentSizer = new wxBoxSizer(wxVERTICAL);

			// Diode Checkboxes
			std::vector<wxCheckBox*> diodeCheckboxes;
			wxBoxSizer* diodeCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			vector<int> diodeIDs = lc->GetLddIds();
			for (int id : diodeIDs) {
				std::string diodeLabel = lc->GetLDDLabel(id);
				wxCheckBox* diodeCheckBox = new wxCheckBox(diodeContentPanel, wxID_ANY, diodeLabel, wxDefaultPosition, wxDefaultSize);
				diodeCheckBox->SetValue(true);

				diodeCheckBox->Bind(wxEVT_CHECKBOX, [&, diodePlot](wxCommandEvent& event) {
					if (diodePlot) {
						diodePlot->RefreshGraph();
					}
					});

				diodeCheckboxes.push_back(diodeCheckBox);
				diodeCheckboxSizer->Add(diodeCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}
			diodeContentSizer->Add(diodeCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			// Diode Graph
			diodePlot = new GraphPlotting(diodeContentPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), diodeCheckboxes);
			diodeContentSizer->Add(diodePlot, 1, wxEXPAND | wxALL, 5);

			diodeContentPanel->SetSizer(diodeContentSizer);
			diodeSizer->Add(diodeContentPanel, 1, wxEXPAND | wxALL, 5);

			// Initially hide the content panel and set toggle button to "closed" state
			diodeContentPanel->Hide();
			diodeToggleButton->SetBitmap(expandBitmap);

			// Diode Toggle Button Event
			diodeToggleButton->Bind(wxEVT_BUTTON, [diodeContentPanel, diodeToggleButton, scrolledWindow, scrollSizer, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = diodeContentPanel->IsShown();
				diodeContentPanel->Show(!isVisible);
				diodeToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);


				scrollSizer->Layout();
				scrolledWindow->FitInside();
				scrolledWindow->Refresh();
				scrolledWindow->Update();
				});


			RealTimeObserver* diodeObserver = new RealTimeObserver(
				RealTimeTempLogTextCtrl,
				diodePlot,
				PlotType::Diode,
				diodeContentPanel,
				diodeToggleButton,
				diodeTogglePanel,
				diodeUpdateLayout
			);
			logger->addObserver(diodeObserver);
			


			diodePanel->SetSizer(diodeSizer);
			scrollSizer->Add(diodePanel, 0, wxEXPAND | wxALL, 5);


			scrolledWindow->SetSizer(scrollSizer);
			scrolledWindow->FitInside();
			scrolledWindow->SetScrollRate(10, 10);



			// ******** Power Panel ******//
			wxPanel* powerPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* powerSizer = new wxBoxSizer(wxVERTICAL);

			GraphPlotting* powerPlot = nullptr;

			auto powerUpdateLayout = [scrolledWindow, powerPanel]() {
				scrolledWindow->Layout();
				scrolledWindow->FitInside();
				powerPanel->Layout();
				};


			// Power Toggle Button
			wxPanel* powerTogglePanel = new wxPanel(powerPanel, wxID_ANY);
			wxBoxSizer* powerToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* powerToggleLabel = new wxStaticText(powerTogglePanel, wxID_ANY, _("Power Monitor"), wxDefaultPosition, wxDefaultSize);
			powerToggleLabel->SetFont(powerToggleLabel->GetFont().Bold());
			wxBitmapButton* powerToggleButton = new wxBitmapButton(powerTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			powerToggleSizer->Add(powerToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			powerToggleSizer->Add(powerToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			powerTogglePanel->SetSizer(powerToggleSizer);
			powerSizer->Add(powerTogglePanel, 0, wxEXPAND | wxALL, 5);

			// Power Panel Content
			wxPanel* powerContentPanel = new wxPanel(powerPanel, wxID_ANY);
			wxBoxSizer* powerContentSizer = new wxBoxSizer(wxVERTICAL);

			// Power Checkboxes
			std::vector<wxCheckBox*> powerCheckboxes;
			wxBoxSizer* powerCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			vector<int> powerMonitorIDs = lc->GetPowerMonitorIDs();
			//wxLogMessage("PowerMonitorIDs size: %zu", powerMonitorIDs.size());
			for (int id : powerMonitorIDs) {
				std::string label = lc->GetPowerMonitorLabel(id);
				//wxLogMessage("PowerMonitor ID: %d, Label: %s", id, label.c_str());
				wxCheckBox* powerCheckBox = new wxCheckBox(powerContentPanel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
				powerCheckBox->SetValue(true);

				powerCheckBox->Bind(wxEVT_CHECKBOX, [&, powerPlot](wxCommandEvent& event) {
					if (powerPlot) {
						powerPlot->RefreshGraph();
					}
					});

				powerCheckboxes.push_back(powerCheckBox);
				powerCheckboxSizer->Add(powerCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
				//wxLogMessage("PowerCheckboxes size: %zu", powerCheckboxes.size());
			}
			powerContentSizer->Add(powerCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			// Power Graph
			powerPlot = new GraphPlotting(powerContentPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), powerCheckboxes);
			powerContentSizer->Add(powerPlot, 1, wxEXPAND | wxALL, 5);

			powerContentPanel->SetSizer(powerContentSizer);
			powerSizer->Add(powerContentPanel, 1, wxEXPAND | wxALL, 5);

			// Initially hide the content panel and set toggle button to "closed" state
			powerContentPanel->Hide();
			powerToggleButton->SetBitmap(expandBitmap);

			// Power Toggle Button Event
			powerToggleButton->Bind(wxEVT_BUTTON, [powerContentPanel, powerToggleButton, scrolledWindow, scrollSizer, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = powerContentPanel->IsShown();
				powerContentPanel->Show(!isVisible);
				powerToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);

				// Correctly adjust the layout dynamically
				scrollSizer->Layout();
				scrolledWindow->FitInside();
				scrolledWindow->Refresh();
				scrolledWindow->Update();
				});

			RealTimeObserver* powerObserver = new RealTimeObserver(
				RealTimeTempLogTextCtrl,
				powerPlot,
				PlotType::Power,
				powerContentPanel,
				powerToggleButton,
				powerTogglePanel,
				powerUpdateLayout
			);
			logger->addObserver(powerObserver);
			powerPanel->SetSizer(powerSizer);
			scrollSizer->Add(powerPanel, 0, wxEXPAND | wxALL, 5); // Add panel without fixed proportion

			// Ensure scrolled window layout adjusts dynamically
			scrolledWindow->SetSizer(scrollSizer);
			scrolledWindow->FitInside();
			scrolledWindow->SetScrollRate(10, 10);



			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			wxPanel* sensorPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* sensorSizer = new wxBoxSizer(wxVERTICAL);

			GraphPlotting* sensorPlot = nullptr;

			auto sensorUpdateLayout = [scrolledWindow, sensorPanel]() {
				scrolledWindow->Layout();
				scrolledWindow->FitInside();
				sensorPanel->Layout();
				};



			wxPanel* sensorTogglePanel = new wxPanel(sensorPanel, wxID_ANY);
			wxBoxSizer* sensorToggleSizer = new wxBoxSizer(wxHORIZONTAL);
			wxStaticText* sensorToggleLabel = new wxStaticText(sensorTogglePanel, wxID_ANY, _("Sensors"), wxDefaultPosition, wxDefaultSize);
			sensorToggleLabel->SetFont(sensorToggleLabel->GetFont().Bold());
			wxBitmapButton* sensorToggleButton = new wxBitmapButton(sensorTogglePanel, wxID_ANY, expandBitmap, wxDefaultPosition, wxSize(30, 30));
			sensorToggleSizer->Add(sensorToggleButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			sensorToggleSizer->Add(sensorToggleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			sensorTogglePanel->SetSizer(sensorToggleSizer);
			sensorSizer->Add(sensorTogglePanel, 0, wxEXPAND | wxALL, 5);

			// Sensor Panel Content
			wxPanel* sensorContentPanel = new wxPanel(sensorPanel, wxID_ANY);
			wxBoxSizer* sensorContentSizer = new wxBoxSizer(wxVERTICAL);

			// Create checkboxes for Flow and Humidity
			std::vector<wxCheckBox*> sensorCheckboxes;
			wxBoxSizer* sensorCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			if (lc->ChillerFlowIsEnabledForUse()) {
				wxCheckBox* flowCheckbox = new wxCheckBox(sensorContentPanel, wxID_ANY, "Chiller Flow", wxDefaultPosition, wxDefaultSize);
				flowCheckbox->SetValue(true);
				sensorCheckboxes.push_back(flowCheckbox);
				sensorCheckboxSizer->Add(flowCheckbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}

			auto humidityIds = lc->GetHumidityIds();
			for (int id : humidityIds) {
				std::string humidityLabel = lc->GetHumidityLabel(id);
				wxCheckBox* humidityCheckbox = new wxCheckBox(sensorContentPanel, wxID_ANY, humidityLabel, wxDefaultPosition, wxDefaultSize);
				humidityCheckbox->SetValue(true);
				sensorCheckboxes.push_back(humidityCheckbox);
				sensorCheckboxSizer->Add(humidityCheckbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}

			// Add checkboxes under the sensor plot
			sensorContentSizer->Add(sensorCheckboxSizer, 0, wxEXPAND | wxALL, 5);


			sensorPlot = new GraphPlotting(sensorContentPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), sensorCheckboxes);
			sensorPlot->SetMinSize(wxSize(1000, 300));
			sensorContentSizer->Add(sensorPlot, 1, wxEXPAND | wxALL, 5);

			sensorContentPanel->SetSizer(sensorContentSizer);
			sensorSizer->Add(sensorContentPanel, 1, wxEXPAND | wxALL, 5);


			sensorContentPanel->Hide();
			sensorToggleButton->SetBitmap(expandBitmap);

			// Sensor Toggle Button Event
			sensorToggleButton->Bind(wxEVT_BUTTON, [sensorContentPanel, sensorToggleButton, scrolledWindow, scrollSizer, expandBitmap, collapseBitmap](wxCommandEvent& event) {
				bool isVisible = sensorContentPanel->IsShown();
				sensorContentPanel->Show(!isVisible);
				sensorToggleButton->SetBitmap(isVisible ? expandBitmap : collapseBitmap);


				scrollSizer->Layout();
				scrolledWindow->FitInside();
				scrolledWindow->Refresh();
				scrolledWindow->Update();
				});

			RealTimeObserver* sensorObserver = new RealTimeObserver(
				RealTimeTempLogTextCtrl,
				sensorPlot,
				PlotType::Sensors,
				sensorContentPanel,
				sensorToggleButton,
				sensorTogglePanel,
				sensorUpdateLayout
			);
			logger->addObserver(sensorObserver);

			sensorPanel->SetSizer(sensorSizer);
			scrollSizer->Add(sensorPanel, 0, wxEXPAND | wxALL, 5);


			scrolledWindow->SetSizer(scrollSizer);
			scrolledWindow->FitInside();
			scrolledWindow->SetScrollRate(10, 10);




			// --------------------------------------------------------------------------------------------------------------------------------------------------------

			scrolledWindow->SetSizer(scrollSizer);
			scrolledWindow->FitInside();
			scrolledWindow->SetScrollRate(10, 10);

			mainSizer->Add(scrolledWindow, 1, wxEXPAND | wxALL, 5);
			mainPanel->SetSizer(mainSizer);
			mainPanel->FitInside();

			wxTimer* alarmCheckTimer = new wxTimer(this);
			Bind(wxEVT_TIMER, [&, mainPanel, observer](wxTimerEvent&) {
				if (observer->IsAlarmTriggered()) {
					wxClientDC dc(mainPanel);
					dc.SetPen(wxPen(wxColour(255, 0, 0), 2));
					dc.DrawLine(100, 0, 100, mainPanel->GetSize().GetHeight());
				}
				}, alarmCheckTimer->GetId());
			alarmCheckTimer->Start(1000);


			graphWindow->Show();

			RefreshControlsEnabled();
			LOG_ACTION()
		}
}


//GraphingWork graphingWork(this, logger, RealTimeTempLogTextCtrl, lc); // Pass RealTimeTempLogTextCtrl
//graphingWork.CreateGraphWindow();



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
}


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
