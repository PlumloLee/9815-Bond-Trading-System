/**
* tradebookingservice.hpp
* Defines the data types and Service for trade booking.
 * This should read data from trades.txt (again, with a separate process reading from the file)
 * publishing via socket into the trading system, which populates via a Connector into the BondTradeBookingService).
*
* @author Breman Thuraisingham
*/
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "executionservice.hpp"
#include "products.hpp"
#include "functions.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
* Trade object with a price, side, and quantity on a particular book.
* Type T is the product type.
*/
template<typename T>
class Trade{
public:

	// ctor for a trade
	Trade() = default;
	Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side){
        tradeId = _tradeId;
        price = _price;
        book = _book;
        quantity = _quantity;
        side = _side;
    }

	// Get the product
	const T& GetProduct() const{
        return product;}
	// Get the trade ID
	const string& GetTradeId() const{
        return tradeId;}
	// Get the mid price
	double GetPrice() const{
        return price;}
	// Get the book
	const string& GetBook() const{
        return book;}
	// Get the quantity
	long GetQuantity() const{
        return quantity;}
	// Get the side
	Side GetSide() const{
        return side;}
private:
	T product;
	string tradeId;
	double price;
	string book;
	long quantity;
	Side side;
};


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingListenerFromExecution;

/**
* Trade Booking Service to book trades to a particular book.
* Keyed on trade identifier.
* Type T is the product type.
*/
template<typename T>
class TradeBookingService : public Service<string, Trade<T>>
{
private:
	map<string, Trade<T>> trades;
	vector<ServiceListener<Trade<T>>*> listeners;
	TradeBookingConnector<T>* connector;
	TradeBookingListenerFromExecution<T>* listener;
public:
	// Constructor and destructor
	TradeBookingService(){
        trades = map<string, Trade<T>>();
        listeners = vector<ServiceListener<Trade<T>>*>();
        connector = new TradeBookingConnector<T>(this);
        listener = new TradeBookingListenerFromExecution<T>(this);
    }
	~TradeBookingService() = default;


	// Get data on our service given a key
	Trade<T>& GetData(string _key) override{
        return trades[_key];
    }
	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Trade<T>& _data) override{
        trades[_data.GetTradeId()] = _data;
        for (auto& l : listeners){
            l->ProcessAdd(_data);
        }
    }
	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Trade<T>>* _listener) override{
        listeners.push_back(_listener);
    }
	// Get all listeners on the Service
	const vector<ServiceListener<Trade<T>>*>& GetListeners() const override{
        return listeners;
    }
	// Get the connector of the service
	TradeBookingConnector<T>* GetConnector(){
        return connector;
    }
	// Get the listener of the service
	TradeBookingListenerFromExecution<T>* GetListener(){
        return listener;
    }
};


/**
* Trade Booking Connector subscribing data to Trading Booking Service.
* Type T is the product type.
*/
template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{
private:
	TradeBookingService<T>* service;
public:
	// Connector and Destructor
	TradeBookingConnector(TradeBookingService<T>* _service){
        service = _service;
    }
	~TradeBookingConnector() = default;

	// Publish data to the Connector
	void Publish(Trade<T>& _data){}

    // Subscribe data from the Connector
    void Subscribe(ifstream& _data) {
        string _line;
        while (getline(_data, _line)) {
            stringstream _lineStream(_line);
            string _cell;
            vector<string> _cells;

            // Efficiently reserve space for the expected number of cells
            _cells.reserve(6);  // Assuming 6 fields per line

            while (getline(_lineStream, _cell, ',')) {
                _cells.push_back(std::move(_cell));  // Use move semantics to avoid copying strings
            }

            // Directly process the data without intermediate variables where possible
            Side _side = (_cells[5] == "BUY") ? BUY : SELL;
            T _product = GetBond(_cells[0]);

            // Create and process the trade
            Trade<T> _trade(_product, _cells[1], ConvertPrice(_cells[2]), _cells[3], stol(_cells[4]), _side);
            service->OnMessage(_trade);
        }
    }
};

/**
* Trade Booking Service Listener subscribing data from Execution Service to Trading Booking Service.
* Type T is the product type.
*/
template<typename T>
class TradeBookingListenerFromExecution : public ServiceListener<ExecutionOrder<T>>
{
private:
	TradeBookingService<T>* service;
	long count;
public:
	// Connector and Destructor
	TradeBookingListenerFromExecution(TradeBookingService<T>* _service){
        service = _service;
        count = 0;}
	~TradeBookingListenerFromExecution() = default;
	// Listener callback to process an add event to the Service
    // Listener callback to process an add event to the Service
    void ProcessAdd(ExecutionOrder<T>& _data) override {
        // Increment the trade count
        count++;

        // Simplify the conversion from PricingSide to Side
        Side _side = (_data.GetPricingSide() == BID) ? SELL : BUY;

        // Determine the book based on the current count
        static const vector<string> books = {"TRSY1", "TRSY2", "TRSY3"};
        string _book = books[count % books.size()];

        // Calculate total quantity
        long _quantity = _data.GetVisibleQuantity() + _data.GetHiddenQuantity();

        // Create and process the trade
        Trade<T> _trade(_data.GetProduct(), _data.GetOrderId(), _data.GetPrice(), _book, _quantity, _side);
        service->OnMessage(_trade);
    }

    // Listener callback to process a remove; event to the Service
	void ProcessRemove(ExecutionOrder<T>& _data) override{
    }

	// Listener callback to process an update event to the Service
	void ProcessUpdate(ExecutionOrder<T>& _data) override{
    }
};

#endif
