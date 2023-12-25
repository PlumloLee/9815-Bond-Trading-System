/**
* positionservice.hpp
* Defines the data types and Service for positions.
* The BondPositionService does not need a Connector since data should flow via ServiceListener from the BondTradeBookingService.
* The BondPositionService should be linked to a BondRiskService via a ServiceListener and send all positions to the BondRiskService via the AddPosition() method
* (note that the BondPositionService should not have an explicit reference to the BondRiskService though or versa – link them through a ServiceListener).

* @author Breman Thuraisingham
*
*/
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
* Position class in a particular book.
* Type T is the product type.
*/
template<typename T>
class Position{
public:

	// ctor for a position
	Position() = default;
	Position(const T& _product){
        product = _product;
        positions_all_book = map<string, long>();
    }
	const T& GetProduct() const{
        return product;
    }
	long GetPosition(string& _book){
        return positions_all_book[_book];
    }
	map<string, long> GetPositions(){
        return positions_all_book;
    }
	void AddPosition(const string& _book, long _position){
        positions_all_book[_book] += _position;
    }
	long GetAggregatePosition(){
        long aggregatePosition = 0;
        for (auto& p : positions_all_book){
            aggregatePosition += p.second;
        }
        return aggregatePosition;
    }
	vector<string> ToStrings() const{
        string _product = product.GetProductId();
        vector<string> _positions;
        for (auto& p : positions_all_book){
            string _book = p.first;
            string _position = to_string(p.second);
            _positions.push_back(_book);
            _positions.push_back(_position);
        }

        vector<string> _strings;
        _strings.push_back(_product);
        _strings.insert(_strings.end(), _positions.begin(), _positions.end());
        return _strings;
    }
private:
	T product;
	map<string, long> positions_all_book;  //book_id, position
};


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class PositionListenerFromTradeBooking;

/**
* Position Service to manage positions across multiple books and secruties.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class PositionService: public Service<string, Position<T>>{
private:
	map<string, Position<T>> PidPositionMap; // product_id, position
	vector<ServiceListener<Position<T>>*> listeners;
	PositionListenerFromTradeBooking<T>* listener;
public:
	// Constructor and destructor
	PositionService() {
        listener = new PositionListenerFromTradeBooking<T>(this);
    }
	~PositionService()= default;

	// Get data on our service given a key
	Position<T>& GetData(string _key){
        return PidPositionMap[_key];
    }
	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Position<T>& _data){
        PidPositionMap[_data.GetProduct().GetProductId()] = _data;
        for (auto& _listener : listeners){
            _listener->ProcessAdd(_data);
        }
    }

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Position<T>>* _listener){
        listeners.push_back(_listener);
    }
	// Get all listeners on the Service
	const vector<ServiceListener<Position<T>>*>& GetListeners() const{
        return listeners;
    }
	// Get the listener of the service
	PositionListenerFromTradeBooking<T>* GetListener(){
        return listener;
    }
	// Add a trade to the service
    void AddTrade(const Trade<T>& _trade) {
        const T& _product = _trade.GetProduct();
        const string& _productId = _product.GetProductId();
        long _tradeQuantity = (_trade.GetSide() == BUY) ? _trade.GetQuantity() : -_trade.GetQuantity();

        // Check if the position exists, create if not
        auto it = PidPositionMap.find(_productId);
        if (it == PidPositionMap.end()) {
            // Handling for new product
            Position<T> newPosition(_product);
            it = PidPositionMap.insert(std::make_pair(_productId, newPosition)).first;
        }

        // Update the position for the specific product
        Position<T>& _position = it->second;
        _position.AddPosition(_trade.GetBook(), _tradeQuantity);

        // On_message
        OnMessage(_position);

    }

};

/**
* The BondPositionService does not need a Connector since data should flow via ServiceListener from the BondTradeBookingService.
* Position Service Listener subscribing data from Trading Booking Service to Position Service.
* Type T is the product type.
*/
template<typename T>
class PositionListenerFromTradeBooking : public ServiceListener<Trade<T>>{
private:
	PositionService<T>* service;

public:

	// Connector and Destructor
	PositionListenerFromTradeBooking(PositionService<T>* _service){
        service = _service;
    }
	~PositionListenerFromTradeBooking() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(Trade<T>& _data){
        service->AddTrade(_data);
    }

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Trade<T>& _data){
        // Do nothing
    }

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Trade<T>& _data){
        // Do nothing
    }

};

#endif
