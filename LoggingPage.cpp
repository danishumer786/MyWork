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
			wxScrolledWindow* scrolledWindow = new wxScrolledWindow(graphWindow, wxID_ANY);
			scrolledWindow->SetScrollRate(10, 10);  // Set the scroll rate for the window

			wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

			// --------------------------------------------------------------------------------------------------------------------------------------------------------
			// Create and add the alarm panel at the TOP of the scrolling window
			wxPanel* alarmPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* alarmSizer = new wxBoxSizer(wxHORIZONTAL);

			// Create a static text to display alarm messages
			wxTextCtrl* alarmTextCtrl = new wxTextCtrl(alarmPanel, wxID_ANY, _("No alarms"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

			// Add the text control to the alarm panel sizer
			alarmSizer->Add(alarmTextCtrl, 1, wxEXPAND | wxALL, 5);
			alarmPanel->SetSizer(alarmSizer);

			RealTimeObserver* observer = new RealTimeObserver(RealTimeTempLogTextCtrl, alarmTextCtrl);
			logger->addObserver(observer);

			// --------------------------------------------------------------------------------------------------------------------------------------------------------
			// ******** TEC Panel ********
			wxPanel* tecPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* tecSizer = new wxBoxSizer(wxVERTICAL);
			std::vector<wxCheckBox*> tecCheckboxes;
			wxBoxSizer* tecCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			GraphPlotting* currentPlot = nullptr;
			GraphPlotting* voltagePlot = nullptr;
			GraphPlotting* tempPlot = nullptr;

			// TEC Checkboxes for current, voltage, and temperature
			vector<int> tecIDs = lc->GetTemperatureControlIDs();
			for (int id : tecIDs) {
				std::string label = lc->GetTemperatureControlLabel(id);
				wxCheckBox* tecCheckBox = new wxCheckBox(tecPanel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);

				tecCheckBox->Bind(wxEVT_CHECKBOX, [&, currentPlot, voltagePlot, tempPlot](wxCommandEvent& event) {
					if (currentPlot) { currentPlot->RefreshGraph(); }
					if (voltagePlot) { voltagePlot->RefreshGraph(); }
					if (tempPlot) { tempPlot->RefreshGraph(); }
					});

				tecCheckboxes.push_back(tecCheckBox);
				tecCheckboxSizer->Add(tecCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}
			// Add checkboxes under the current plot
			tecSizer->Add(tecCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			// 1. Create TEC Current label and plot
			wxStaticText* tecCurrentLabel = new wxStaticText(tecPanel, wxID_ANY, _("TEC Current"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			tecCurrentLabel->SetFont(tecCurrentLabel->GetFont().Bold());
			tecSizer->Add(tecCurrentLabel, 0, wxEXPAND | wxALL, 5);  // Add the label above the current graph

			currentPlot = new GraphPlotting(tecPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			currentPlot->SetMinSize(wxSize(1000, 300));
			tecSizer->Add(currentPlot, 1, wxEXPAND | wxALL, 5);  // Add the current graph below the label

			// 2. Create TEC Voltage label and plot
			wxStaticText* tecVoltageLabel = new wxStaticText(tecPanel, wxID_ANY, _("TEC Voltage"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			tecVoltageLabel->SetFont(tecVoltageLabel->GetFont().Bold());
			tecSizer->Add(tecVoltageLabel, 0, wxEXPAND | wxALL, 5);  // Add the label above the voltage graph

			voltagePlot = new GraphPlotting(tecPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			voltagePlot->SetMinSize(wxSize(1000, 300));
			tecSizer->Add(voltagePlot, 1, wxEXPAND | wxALL, 5);  // Add the voltage graph below the label

			// 3. Create TEC Temperature label and plot
			wxStaticText* tecTempLabel = new wxStaticText(tecPanel, wxID_ANY, _("TEC Temperature"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			tecTempLabel->SetFont(tecTempLabel->GetFont().Bold());
			tecSizer->Add(tecTempLabel, 0, wxEXPAND | wxALL, 5);  // Add the label above the temperature graph

			tempPlot = new GraphPlotting(tecPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), tecCheckboxes);
			tempPlot->SetMinSize(wxSize(1000, 300));
			tecSizer->Add(tempPlot, 1, wxEXPAND | wxALL, 10);  // Add the temperature graph below the label

			// Observer for TEC data
			RealTimeObserver* tecObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, currentPlot, voltagePlot, tempPlot);
			logger->addObserver(tecObserver);

			tecPanel->SetSizer(tecSizer);

			// -------------------------------------------------------------------------------------------------------------------------------------------------------
			// ******** Diode Panel ********
			wxPanel* diodePanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* diodeSizer = new wxBoxSizer(wxVERTICAL);

			// Create Diode Current label and plot
			wxStaticText* diodeCurrentLabel = new wxStaticText(diodePanel, wxID_ANY, _("Diode Current"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			diodeCurrentLabel->SetFont(diodeCurrentLabel->GetFont().Bold());
			diodeSizer->Add(diodeCurrentLabel, 0, wxEXPAND | wxALL, 5);  // Add the label above the diode current graph

			std::vector<wxCheckBox*> diodeCheckboxes;
			wxBoxSizer* diodeCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			vector<int> diodeIDs = lc->GetLddIds();  // Assuming diode IDs come from GetLddIds()
			GraphPlotting* diodePlot = nullptr;

			// Diode Checkboxes for currents
			for (int id : diodeIDs) {
				std::string diodeLabel = lc->GetLDDLabel(id);
				wxCheckBox* diodeCheckBox = new wxCheckBox(diodePanel, wxID_ANY, diodeLabel, wxDefaultPosition, wxDefaultSize);

				diodeCheckBox->Bind(wxEVT_CHECKBOX, [&, diodePlot](wxCommandEvent& event) {
					if (diodePlot) {
						diodePlot->RefreshGraph();
					}
					});

				diodeCheckboxes.push_back(diodeCheckBox);
				diodeCheckboxSizer->Add(diodeCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}
			// Add checkboxes under the diode current plot
			diodeSizer->Add(diodeCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			diodePlot = new GraphPlotting(diodePanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), diodeCheckboxes);
			diodeSizer->Add(diodePlot, 1, wxEXPAND | wxALL, 5);  // Add the diode current graph below the label

			diodePanel->SetSizer(diodeSizer);

			// Add observer for diode data
			RealTimeObserver* diodeObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, diodePlot, PlotType::Diode);
			logger->addObserver(diodeObserver);

			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			//Working on Laser Power
			// Add power panel to the main graph window in the LoggingPage::OnStartButtonClicked function
			wxPanel* powerPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* powerSizer = new wxBoxSizer(wxVERTICAL);

			// Create Power Monitor label and plot
			wxStaticText* powerLabel = new wxStaticText(powerPanel, wxID_ANY, _("Power Monitor"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			powerLabel->SetFont(powerLabel->GetFont().Bold());
			powerSizer->Add(powerLabel, 0, wxEXPAND | wxALL, 5);

			std::vector<wxCheckBox*> powerCheckboxes;
			wxBoxSizer* powerCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			// Create Power Checkboxes
			vector<int> powerMonitorIDs = lc->GetPowerMonitorIDs();
			GraphPlotting* powerPlot = nullptr;
			for (int id : powerMonitorIDs) {
				std::string label = lc->GetPowerMonitorLabel(id);
				wxCheckBox* powerCheckBox = new wxCheckBox(powerPanel, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);

				powerCheckBox->Bind(wxEVT_CHECKBOX, [&, powerPlot](wxCommandEvent& event) {
					if (powerPlot) {
						powerPlot->RefreshGraph();
					}
					});

				powerCheckboxes.push_back(powerCheckBox);
				powerCheckboxSizer->Add(powerCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}
			powerSizer->Add(powerCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			// Create Power Graph Plotting
			powerPlot = new GraphPlotting(powerPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), powerCheckboxes);
			//powerPlot->SetMinSize(wxSize(1000, 300));
			powerSizer->Add(powerPlot, 1, wxEXPAND | wxALL, 5);

			powerPanel->SetSizer(powerSizer);

			// Add observer for power data
			RealTimeObserver* powerObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, powerPlot, PlotType::Power);
			logger->addObserver(powerObserver);

			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	        // ******** Sensor Panel ********
			wxPanel* sensorPanel = new wxPanel(scrolledWindow, wxID_ANY);
			wxBoxSizer* sensorSizer = new wxBoxSizer(wxVERTICAL);

			// Create Chiller Flow and Humidity label and plot
			wxStaticText* sensorLabel = new wxStaticText(sensorPanel, wxID_ANY, _("Sensors"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			sensorLabel->SetFont(sensorLabel->GetFont().Bold());
			sensorSizer->Add(sensorLabel, 0, wxEXPAND | wxALL, 5);  // Add the label above the sensor graph

			// Create checkboxes for Flow and Humidity
			std::vector<wxCheckBox*> sensorCheckboxes;
			wxBoxSizer* sensorCheckboxSizer = new wxBoxSizer(wxHORIZONTAL);

			if (lc->ChillerFlowIsEnabledForUse()) {
				wxCheckBox* flowCheckbox = new wxCheckBox(sensorPanel, wxID_ANY, "Chiller Flow", wxDefaultPosition, wxDefaultSize);
				sensorCheckboxes.push_back(flowCheckbox);
				sensorCheckboxSizer->Add(flowCheckbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}

			auto humidityIds = lc->GetHumidityIds();
			for (int id : humidityIds) {
				std::string humidityLabel = lc->GetHumidityLabel(id);
				wxCheckBox* humidityCheckbox = new wxCheckBox(sensorPanel, wxID_ANY, humidityLabel, wxDefaultPosition, wxDefaultSize);
				sensorCheckboxes.push_back(humidityCheckbox);
				sensorCheckboxSizer->Add(humidityCheckbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
			}

			// Add checkboxes under the sensor plot
			sensorSizer->Add(sensorCheckboxSizer, 0, wxEXPAND | wxALL, 5);

			// Create the GraphPlotting instance for sensors
			GraphPlotting* sensorPlot = new GraphPlotting(sensorPanel, wxID_ANY, wxDefaultPosition, wxSize(1000, 200), sensorCheckboxes);
			sensorPlot->SetMinSize(wxSize(1000, 300));
			sensorSizer->Add(sensorPlot, 1, wxEXPAND | wxALL, 5);  // Add the sensor graph below the label

			// Set sizer for sensorPanel
			sensorPanel->SetSizer(sensorSizer);

			// Add observer for sensor data
			RealTimeObserver* sensorObserver = new RealTimeObserver(RealTimeTempLogTextCtrl, sensorPlot, PlotType::Sensors);
			logger->addObserver(sensorObserver);



			// Add both TEC and Diode panels to the mainSizer
			mainSizer->Add(alarmPanel, 0, wxEXPAND | wxALL, 5);
			mainSizer->Add(tecPanel, 3, wxEXPAND | wxALL, 5);
			mainSizer->Add(diodePanel, 1.2, wxEXPAND | wxALL, 5);
			mainSizer->Add(powerPanel, 1, wxEXPAND | wxALL, 5);
			mainSizer->Add(sensorPanel, 1.2, wxEXPAND | wxALL, 5);
			


			// Set the mainSizer to the scrolled window
			scrolledWindow->SetSizer(mainSizer);
			scrolledWindow->FitInside();

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
