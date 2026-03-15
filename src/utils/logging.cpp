#include<bits/stdc++.h>
#include"logging.h"
using namespace std;

Logging log1;

void logging_init(string filename, Logging_Levels level) {
	std::ios::sync_with_stdio(false);
	log1.logfile.open(filename, ios::app);
	log1.loglevel=level;
}

void logging(Logging_Levels level, string message) {
	if (level<log1.loglevel) return;
	time_t current_time = time({});
    char timeString[std::size("[yyyy-mm-ddThh:mm:ss]")];
    strftime(std::data(timeString), std::size(timeString), "[%F %T]", std::localtime(&current_time));
	cout << timeString << ' ' << message << endl;
	log1.logfile << timeString << ' ' << message << endl;
}