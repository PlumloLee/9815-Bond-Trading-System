/**
* algostreamingservice.hpp
* Defines the data types and Service for algo streamings.
*
* @author Breman Thuraisingham
*/
#ifndef ALGO_STREAMING_SERVICE_HPP
#define ALGO_STREAMING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "pricingservice.hpp"

/**
* A price stream order with price and quantity (visible and hidden)
*/
class PriceStreamOrder{
public:
	PriceStreamOrder() = default; // ctor for an order
	PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side){ // ctor for an order
        price = _price;
        visibleQuantity = _visibleQuantity;
        hiddenQuantity = _hiddenQuantity;
        side = _side;
    }
	double GetPrice() const{
        // Get the price on this order
        return price;
    }
	long GetVisibleQuantity() const{
        // Get the visible quantity on this order
        return visibleQuantity;
    }
	long GetHiddenQuantity() const{
        // Get the hidden quantity on this order
        return hiddenQuantity;
    }
	PricingSide GetSide() const{
        // The side on this order
        return side;
    }
	vector<string> ToStrings() const{
        // Change attributes to strings
        string _price = ConvertPrice(price);
        return vector<string>{_price, to_string(visibleQuantity), to_string(hiddenQuantity)};
    }
private:
	double price;
	long visibleQuantity;
	long hiddenQuantity;
	PricingSide side;

};

/**
* Price Stream with a two-way market.
* Type T is the product type.
*/
template<typename T>
class PriceStream{
public:
    // Constructor
    PriceStream() = default;
    PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
            : product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder) {}
    // Get the product
    const T& GetProduct() const { return product; }
    // Get the bid order
    const PriceStreamOrder& GetBidOrder() const { return bidOrder; }
    // Get the offer order
    const PriceStreamOrder& GetOfferOrder() const { return offerOrder; }
    // Change attributes to strings
    vector<string> ToStrings() const
    {
        string _product = product.GetProductId();
        vector<string> _bidOrder = bidOrder.ToStrings();
        vector<string> _offerOrder = offerOrder.ToStrings();

        vector<string> _strings;
        _strings.push_back(_product);
        _strings.insert(_strings.end(), _bidOrder.begin(), _bidOrder.end());
        _strings.insert(_strings.end(), _offerOrder.begin(), _offerOrder.end());
        return _strings;
    }

private:
    T product;
    PriceStreamOrder bidOrder;
    PriceStreamOrder offerOrder;
};


/**
* An algo streaming that process algo streaming.
* Type T is the product type.
*/
template<typename T>
class AlgoStream
{
public:
    // Constructor
    AlgoStream() = default;
    AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
    {
        priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
    }
    // Get the order
    PriceStream<T>* GetPriceStream() const { return priceStream; }
private:
    PriceStream<T>* priceStream;
};

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class AlgoStreamingToPricingListener;

/**
 * Service for algo streaming prices.
 * @tparam T
 */
template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{
private:
    map<string, AlgoStream<T>> algoStreams;
    vector<ServiceListener<AlgoStream<T>>*> listeners;
    ServiceListener<Price<T>>* listener;
    long count;

public:
    // Constructor
    AlgoStreamingService() : count(0)
    {
        algoStreams = map<string, AlgoStream<T>>();
        listeners = vector<ServiceListener<AlgoStream<T>>*>();
        listener = new AlgoStreamingToPricingListener<T>(this);
    }
    // Destructor
    ~AlgoStreamingService() {}
    // Get data on our service given a key
    AlgoStream<T>& GetData(string _key)
    {
        return algoStreams[_key];
    }
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(AlgoStream<T>& _data)
    {
        algoStreams[_data.GetPriceStream()->GetProduct().GetProductId()] = _data;
    }
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoStream<T>>* _listener)
    {
        listeners.push_back(_listener);
    }
    // Get all listeners on the Service
    const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const
    {
        return listeners;
    }
    // Get the listener of the service
    ServiceListener<Price<T>>* GetListener()
    {
        return listener;
    }
    // Publish two-way prices
    void AlgoPublishPrice(Price<T>& _price)
    {
        T _product = _price.GetProduct();
        string _productId = _product.GetProductId();

        double _mid = _price.GetMid();
        double _bidOfferSpread = _price.GetBidOfferSpread();
        double _bidPrice = _mid - _bidOfferSpread / 2.0;
        double _offerPrice = _mid + _bidOfferSpread / 2.0;
        long _visibleQuantity = (count % 2 + 1) * 10000000;
        long _hiddenQuantity = _visibleQuantity * 2;

        count++;
        PriceStreamOrder _bidOrder(_bidPrice, _visibleQuantity, _hiddenQuantity, BID);
        PriceStreamOrder _offerOrder(_offerPrice, _visibleQuantity, _hiddenQuantity, OFFER);
        AlgoStream<T> _algoStream(_product, _bidOrder, _offerOrder);
        algoStreams[_productId] = _algoStream;


        for (auto& l : listeners)
        {
            l->ProcessAdd(_algoStream);
        }
    }
};

/**
* Algo Streaming Service Listener subscribing data from Pricing Service to Algo Streaming Service.
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingToPricingListener : public ServiceListener<Price<T>>
{
private:
    AlgoStreamingService<T>* service;

public:
    // Constructor
    AlgoStreamingToPricingListener(AlgoStreamingService<T>* _service) : service(_service) {}

    // Destructor
    ~AlgoStreamingToPricingListener() {}

    // Listener callback to process an add event to the Service
    void ProcessAdd(Price<T>& _data)
    {
        service->AlgoPublishPrice(_data);
    }

    // Listener callback to process a remove event to the Service
    void ProcessRemove(Price<T>& _data)
    {
        // Implementation for ProcessRemove
    }

    // Listener callback to process an update event to the Service
    void ProcessUpdate(Price<T>& _data)
    {
        // Implementation for ProcessUpdate
    }
};

#endif
