/**
* algoexecutionservice.hpp
* Defines the data types and Service for algo executions.
*
* @author Breman Thuraisingham
*/
#ifndef ALGO_EXECUTION_SERVICE_HPP
#define ALGO_EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
* An execution order that can be placed on an exchange.
* Type T is the product type.
*/
template<typename T>
class ExecutionOrder
{
public:

    ExecutionOrder() = default;// ctor for an order
    ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder)
            : product(_product), side(_side), orderId(_orderId), orderType(_orderType), price(_price), visibleQuantity(_visibleQuantity), hiddenQuantity(_hiddenQuantity), parentOrderId(_parentOrderId), isChildOrder(_isChildOrder)
    {}
    const T& GetProduct() const { return product; } // Get the product
    PricingSide GetPricingSide() const { return side; } // Get the pricing side
    const string& GetOrderId() const { return orderId; } // Get the order ID
    OrderType GetOrderType() const { return orderType; } // Get the order type on this order
    double GetPrice() const { return price; } // Get the price on this order
    long GetVisibleQuantity() const { return visibleQuantity; } // Get the visible quantity on this order
    long GetHiddenQuantity() const { return hiddenQuantity; } // Get the hidden quantity
    const string& GetParentOrderId() const { return parentOrderId; } // Get the parent order ID
    bool IsChildOrder() const { return isChildOrder; } // Is child order?
    vector<string> ToStrings() const
    {
        // Assuming ConvertPrice is a function that converts double to string
        // and GetProductId is a method of the product class

        static const map<PricingSide, string> sideToString = {{BID, "BID"}, {OFFER, "OFFER"}};
        static const map<OrderType, string> orderTypeToString = {
                {FOK, "FOK"}, {IOC, "IOC"}, {MARKET, "MARKET"}, {LIMIT, "LIMIT"}, {STOP, "STOP"}};

        vector<string> strings;
        strings.reserve(9);  // Preallocate space for 9 elements

        strings.push_back(product.GetProductId());
        strings.push_back(sideToString.at(side));
        strings.push_back(orderId);
        strings.push_back(orderTypeToString.at(orderType));
        strings.push_back(ConvertPrice(price));
        strings.push_back(to_string(visibleQuantity));
        strings.push_back(to_string(hiddenQuantity));
        strings.push_back(parentOrderId);
        strings.push_back(isChildOrder ? "YES" : "NO");

        return strings;
    } // Change attributes to strings
private:
    T product;
    PricingSide side;
    string orderId;
    OrderType orderType;
    double price;
    long visibleQuantity;
    long hiddenQuantity;
    string parentOrderId;
    bool isChildOrder;
};

/**
* An algo execution that process algo execution.
* Type T is the product type.
*/
template<typename T>
class AlgoExecution
{
public:
    AlgoExecution() = default; // Constructor
    AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder)
    {
        executionOrder = new ExecutionOrder<T>(_product, _side, _orderId, _orderType, _price, _visibleQuantity, _hiddenQuantity, _parentOrderId, _isChildOrder);
    }
    ExecutionOrder<T>* GetExecutionOrder() const
    {
        return executionOrder;
    } // Get the order
private:
    ExecutionOrder<T>* executionOrder;
};

template<typename T>
class AlgoExecutionListenerFromMarketData;

/**
* Service for algo executing orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{
private:
    map<string, AlgoExecution<T>> algoExecutions;
    vector<ServiceListener<AlgoExecution<T>>*> listeners;
    AlgoExecutionListenerFromMarketData<T>* listener;
    double spread;
    long count;
public:
    AlgoExecutionService()
    {
        algoExecutions = map<string, AlgoExecution<T>>();
        listeners = vector<ServiceListener<AlgoExecution<T>>*>();
        listener = new AlgoExecutionListenerFromMarketData<T>(this);
        spread = 1.0 / 128.0;
        count = 0;
    }  // Constructor
    ~AlgoExecutionService() {} // Destructor
    AlgoExecution<T>& GetData(string _key)
    {
        return algoExecutions[_key];
    } // Get data on our service given a key
    void OnMessage(AlgoExecution<T>& _data)
    {
        algoExecutions[_data.GetExecutionOrder()->GetProduct().GetProductId()] = _data;
    } // The callback that a Connector should invoke for any new or updated data
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoExecution<T>>* _listener)
    {
        listeners.push_back(_listener);
    }
    // Get all listeners on the Service
    const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const
    {
        return listeners;
    }
    // Get the listener of the service
    AlgoExecutionListenerFromMarketData<T>* GetListener()
    {
        return listener;
    }
    // Publish algo streams (called by algo streaming service listener to subscribe data from pricing service)
    void AlgoExecuteOrder(OrderBook<T>& _orderBook)
    {
        T _product = _orderBook.GetProduct();
        string _productId = _product.GetProductId();
        PricingSide _side;
        string _orderId = GenerateId();
        double _price;
        long _quantity;

        BidOffer _bidOffer = _orderBook.GetBidOffer();

        Order _bidOrder = _bidOffer.GetBidOrder();
        double _bidPrice = _bidOrder.GetPrice();
        long _bidQuantity = _bidOrder.GetQuantity();


        Order _offerOrder = _bidOffer.GetOfferOrder();
        double _offerPrice = _offerOrder.GetPrice();
        long _offerQuantity = _offerOrder.GetQuantity();

//        cout << _offerPrice << _bidPrice << endl;
        if (_offerPrice - _bidPrice <= spread)
        {
            switch (count % 2)
            {
                case 0:
                    _price = _bidPrice;
                    _quantity = _bidQuantity;
                    _side = BID;
                    break;
                case 1:
                    _price = _offerPrice;
                    _quantity = _offerQuantity;
                    _side = OFFER;
                    break;
            }
            count++;
            AlgoExecution<T> _algoExecution(_product, _side, _orderId, MARKET, _price, _quantity, 0, "", false);
            algoExecutions[_productId] = _algoExecution;


            for (auto& l : listeners)
            {
                l->ProcessAdd(_algoExecution);
            }
        }
    }
};

/**
* Algo Execution Service Listener subscribing data from Market Data Service to Algo Execution Service.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionListenerFromMarketData : public ServiceListener<OrderBook<T>>{
private:
    AlgoExecutionService<T>* service;
public:
    AlgoExecutionListenerFromMarketData(AlgoExecutionService<T>* _service){
        // Connector and Destructor
        service = _service;
    }
    ~AlgoExecutionListenerFromMarketData(){
        // Destructor
        service = nullptr;
    }
    void ProcessAdd(OrderBook<T>& _data){
        // Listener callback to process an add event to the Service
        service->AlgoExecuteOrder(_data);
    }
    void ProcessRemove(OrderBook<T>& _data){
        // Listener callback to process a remove event to the Service
    }
    void ProcessUpdate(OrderBook<T>& _data){
        // Listener callback to process an update event to the Service
    }
};

#endif