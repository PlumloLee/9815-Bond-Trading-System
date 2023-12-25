/**
* pricingservice.hpp
* Defines the data types and Service for internal prices.
*
*/
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "products.hpp"
#include "functions.hpp"

/**
* A price object consisting of mid and bid/offer spread.
* Type T is the product type.
 * @param <T>
 * @return
*/
template<typename T>
class Price{
public:
	
	Price() = default; // ctor for a price
	Price(const T& _product, double _mid, double _bidOfferSpread): product(_product){
        mid = _mid;
        bidOfferSpread = _bidOfferSpread;
    }
	const T& GetProduct() const{ return product;} // Get the product
	double GetMid() const { return mid; }  // Get the midprice
	double GetBidOfferSpread() const { return bidOfferSpread; } // Get the bid/offer spread around the mid
	vector<string> ToStrings() const{
        string _product = product.GetProductId();
        string _mid = ConvertPrice(mid);
        string _bidOfferSpread = ConvertPrice(bidOfferSpread);
        vector<string> _strings;
        _strings.push_back(_product);
        _strings.push_back(_mid);
        _strings.push_back(_bidOfferSpread);
        return _strings;
    }

private:

	T product;
	double mid;
	double bidOfferSpread;

};



template<typename T>
class PricingConnector;

/**
* Pricing Service managing mid prices and bid/offers.
* Keyed on product identifier.
*/
template<typename T>
class PricingService : public Service<string, Price<T>>{
private:
	map<string, Price<T>> PrdPricesMap;
	vector<ServiceListener<Price<T>>*> listeners;
	PricingConnector<T>* connector;

public:
	// Constructor and destructor
	PricingService(){
        connector = new PricingConnector<T>(this);
    }
	~PricingService() = default;
    
	Price<T>& GetData(string _productId) override{
        return PrdPricesMap[_productId];}
    void OnMessage(Price<T>& _data) override {    	// The callback that a Connector should invoke for any new or updated data
        PrdPricesMap[_data.GetProduct().GetProductId()] = _data;
        for (auto& listener : listeners) {
            listener->ProcessAdd(_data);  
        }
    }
	void AddListener(ServiceListener<Price<T>>* _listener) override{
        listeners.push_back(_listener);}  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	const vector<ServiceListener<Price<T>>*>& GetListeners() const override{
        return listeners;
    }
	PricingConnector<T>* GetConnector(){
        return connector;
    }
};


/**
* Pricing Connector subscribing data to Pricing Service.
* Type T is the product type.
*/
template<typename T>
class PricingConnector : public Connector<Price<T>>{
private:
	PricingService<T>* service;

public:
	PricingConnector(PricingService<T>* _service){
        service = _service;
    }
	~PricingConnector() = default;

	// Publish data to the Connector
	void Publish(Price<T>& _data){}

	// Subscribe data from the Connector
	void Subscribe(ifstream& _data){
        string _line;
        while (getline(_data, _line)){
            stringstream _lineStream(_line);
            string _cell;
            vector<string> _cells;
            while (getline(_lineStream, _cell, ',')){
                _cells.push_back(_cell);
            }

            string _productId = _cells[0];
            double _bidPrice = ConvertPrice(_cells[1]);
            double _offerPrice = ConvertPrice(_cells[2]);
            double _midPrice = (_bidPrice + _offerPrice) / 2.0;
            double _spread = _offerPrice - _bidPrice;
            T _product = GetBond(_productId);
            Price<T> _price(_product, _midPrice, _spread);
            service->OnMessage(_price);
        }
    }

};

#endif
