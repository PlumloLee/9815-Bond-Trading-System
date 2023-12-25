/**
* functions.hpp
* Defines Utility functions for the trading system.
*
*/
#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <iostream>
#include <string>
#include <chrono>
#include "products.hpp"
#include <string>
#include <unordered_map>

using namespace std;
using namespace chrono;

std::map<std::string, std::function<Bond()>> bondCreatorMap = {
        {"9128283H1", []() { return Bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30")); }},
        {"9128283L2", []() { return Bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15"));}},
        {"912828M80", []() { return Bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30")); }},
        {"9128283J7", []() { return Bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30")); }},
        {"9128283F5", []() { return Bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15")); }},
        {"912810TW8", []() { return Bond("912810TW8", CUSIP, "US20Y", 0.02500, from_string("2037/12/15")); }},
        {"912810RZ3", []() { return Bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15")); }}
};

Bond GetBond(const std::string& cusip) {
    auto it = bondCreatorMap.find(cusip);
    if (it != bondCreatorMap.end()) {
        return it->second(); // Call the function to create the Bond object
    } else {
        throw std::invalid_argument("Invalid CUSIP");
    }
}
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// get time and format in milliseconds
string getTime() {
    auto now = chrono::system_clock::now();
    auto now_time_t = chrono::system_clock::to_time_t(now);
    tm now_tm = *localtime(&now_time_t);

    // get the milliseconds component
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // format the time and append milliseconds
    ostringstream oss;
    oss << put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << setfill('0') << setw(3) << milliseconds.count();

    return oss.str();
}
string getTime(chrono::system_clock::time_point _now) {
    auto now_time_t = chrono::system_clock::to_time_t(_now);
    tm now_tm = *localtime(&now_time_t);

    // get the milliseconds component
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(_now.time_since_epoch()) % 1000;

    // format the time and append milliseconds
    ostringstream oss;
    oss << put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << setfill('0') << setw(3) << milliseconds.count();

    return oss.str();
}

// join a vector of string with delimiter
string join(const vector<string>& strings, const string& delimiter) {
    string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }

    return result;
}



// log messages with different levels
void log(LogLevel level, const string& message) {
    string levelStr;
    switch (level) {
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
    }
    cout << getTime() << " [" << levelStr << "] " << message << endl;
}


// Generate uniformly distributed random variables between 0 to 1.
vector<double> GenerateUniform(long N, long seed = 0)
{
	long m = 2147483647;
	long a = 39373;
	long q = m / a;
	long r = m % a;

	if (seed == 0) seed = time(0);
	seed = seed % m;
	vector<double> result;
	for (long i = 0; i < N; i++)
	{
		long k = seed / q;
		seed = a * (seed - k * q) - k * r;
		if (seed < 0) seed = seed + m;
		result.push_back(seed / (double)m);
	}
	return result;
}
static const std::unordered_map<std::string, double> pv01Values = {
    {"9128283H1", 0.01948992},
    {"9128283L2", 0.02865304},
    {"912828M80", 0.04581119},
    {"9128283J7", 0.06127718},
    {"9128283F5", 0.08161449},
    {"912810RZ3", 0.15013155}
};


// Get PV01 value for US Treasury 2Y, 3Y, 5Y, 7Y, 10Y, and 30Y.
double GetPV01Value(const std::string& _cusip) {
    auto it = pv01Values.find(_cusip);
    if (it != pv01Values.end()) {
        return it->second;
    }
    // Return a default value or handle the case where the CUSIP is not found
    return 0.0; // You might want to handle this more appropriately
}



#include <cmath>
#include <string>

// A simplified function to calculate the PV01 of a bond
double GetPV01Value(const std::string& productId, double yield, double maturity, double couponRate, double faceValue) {
    // Constants
    const double BASIS_POINT = 0.0001;
    const int PAYMENTS_PER_YEAR = 2; // Semi-annual by default, adjust as needed

    // Calculate the present value of future coupon payments
    double couponPayment = couponRate * faceValue / PAYMENTS_PER_YEAR;
    double presentValueOfCoupons = 0.0;
    for (int i = 1; i <= maturity * PAYMENTS_PER_YEAR; ++i) {
        presentValueOfCoupons += couponPayment / pow(1 + yield / PAYMENTS_PER_YEAR, i);
    }

    // Calculate the present value of the face value at maturity
    double presentValueOfFaceValue = faceValue / pow(1 + yield, maturity);

    // Total present value of the bond
    double totalPresentValue = presentValueOfCoupons + presentValueOfFaceValue;

    // Calculate the present value of the bond if yield increases by one basis point
    double adjustedYield = yield + BASIS_POINT;
    double adjustedPresentValueOfCoupons = 0.0;
    for (int i = 1; i <= maturity * PAYMENTS_PER_YEAR; ++i) {
        adjustedPresentValueOfCoupons += couponPayment / pow(1 + adjustedYield / PAYMENTS_PER_YEAR, i);
    }
    double adjustedPresentValueOfFaceValue = faceValue / pow(1 + adjustedYield, maturity);
    double adjustedTotalPresentValue = adjustedPresentValueOfCoupons + adjustedPresentValueOfFaceValue;

    // Calculate and return the PV01
    return totalPresentValue - adjustedTotalPresentValue;
}



double ConvertPrice(const string& priceStr) {
    // Split the price string into its integral and fractional parts
    size_t dashPos = priceStr.find('-');
    double integralPart = stod(priceStr.substr(0, dashPos));
    string fractionalPartStr = priceStr.substr(dashPos + 1);

    // Extract xy and z values from the fractional part
    int xy = stoi(fractionalPartStr.substr(0, 2));
    char zChar = fractionalPartStr[2];

    // Convert zChar to its equivalent number, with '+' being 4
    int z = (zChar == '+') ? 4 : zChar - '0';

    // Convert fractional part to decimal
    double fractionalDecimal = xy / 32.0 + z / 256.0;

    return integralPart + fractionalDecimal;
}


// Convert numerical price to fractional price.
string ConvertPrice(double _doublePrice)
{
	int _doublePrice100 = floor(_doublePrice);
	int _doublePrice256 = floor((_doublePrice - _doublePrice100) * 256.0);
	int _doublePrice32 = floor(_doublePrice256 / 8.0);
	int _doublePrice8 = _doublePrice256 % 8;

	string _stringPrice100 = to_string(_doublePrice100);
	string _stringPrice32 = to_string(_doublePrice32);
	string _stringPrice8 = to_string(_doublePrice8);

	if (_doublePrice32 < 10) _stringPrice32 = "0" + _stringPrice32;
	if (_doublePrice8 == 4) _stringPrice8 = "+";

	string _stringPrice = _stringPrice100 + "-" + _stringPrice32 + _stringPrice8;
	return _stringPrice;
}

// Output Time Stamp with millisecond precision.
string TimeStamp()
{
	auto _timePoint = system_clock::now();
	auto _sec = chrono::time_point_cast<chrono::seconds>(_timePoint);
	auto _millisec = chrono::duration_cast<chrono::milliseconds>(_timePoint - _sec);

	auto _millisecCount = _millisec.count();
	string _milliString = to_string(_millisecCount);
	if (_millisecCount < 10) _milliString = "00" + _milliString;
	else if (_millisecCount < 100) _milliString = "0" + _milliString;

	time_t _timeT = system_clock::to_time_t(_timePoint);
	char _timeChar[24];
	strftime(_timeChar, 24, "%F %T", localtime(&_timeT));
	string _timeString = string(_timeChar) + "." + _milliString + " ";

	return _timeString;
}

// Get the millisecond count of current time.
long GetMillisecond()
{
	auto _timePoint = system_clock::now();
	auto _sec = chrono::time_point_cast<chrono::seconds>(_timePoint);
	auto _millisec = chrono::duration_cast<chrono::milliseconds>(_timePoint - _sec);
	long _millisecCount = _millisec.count();
	return _millisecCount;
}

// Generate random IDs.
string GenerateId()
{
	string _base = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
	vector<double> _randoms = GenerateUniform(12, GetMillisecond());
	string _id = "";
	for (auto& r : _randoms)
	{
		int i = r * 36;
		_id.push_back(_base[i]);
	}
	return _id;
}

#endif
