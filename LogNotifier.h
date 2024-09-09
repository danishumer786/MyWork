// James Butcher
// 8/22/24

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#include "LogObserver.h"


class LogNotifier {
//Adding Observers
public:
	void addObserver(LogObserver* observer) {
		logObservers_p.insert(observer);
	};
	void addObserver(std::shared_ptr<LogObserver> observer) {
		logObservers_sp.insert(observer);
	};


protected:
	std::set<std::shared_ptr<LogObserver>> logObservers_sp;
	std::set<LogObserver*> logObservers_p;
	//Notifying Observers
	void dataPointLogged(std::map<std::string, std::string> data) {
		for (auto& observer : logObservers_p)
			observer->onDataPointLogged(data);
		for (auto& observer : logObservers_sp)
			observer->onDataPointLogged(data);
	};

};
