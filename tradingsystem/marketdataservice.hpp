/**
* marketdataservice.hpp
* Defines the data types and Service for order book market data.
*
* @author Breman Thuraisingham
*/
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "products.hpp"
#include "functions.hpp"

using namespace std;
enum PricingSide { BID, OFFER }; // Side for market data
/**
* A market data order with price, quantity, and side.
*/
class Order{
public:
	Order() = default; // ctor for an order
	Order(double _price, long _quantity, PricingSide _side){
        price = _price;
        quantity = _quantity;
        side = _side;
    }
	double GetPrice() const{ return price; } // Get the price on the order
	long GetQuantity() const{ return quantity; }// Get the quantity on the order
	PricingSide GetSide() const{ return side; }// Get the side on the order
private:
	double price;
	long quantity;
	PricingSide side;
};

/**
* Class representing a bid and offer order
*/
class BidOffer{
public:
	BidOffer() = default;// ctor for bid/offer
	BidOffer(const Order& _bidOrder, const Order& _offerOrder){
        bidOrder = _bidOrder;
        offerOrder = _offerOrder;
    }
	const Order& GetBidOrder() const{ // Get the bid order
        return bidOrder;
    };
	const Order& GetOfferOrder() const{ // Get the offer order
        return offerOrder;
    }
private:
	Order bidOrder;
	Order offerOrder;
};

/**
* Order book with a bid and offer stack.
* Type T is the product type.
*/
template<typename T>
class OrderBook{
public:
	// ctor for the order book
	OrderBook() = default;
	OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack){
        product = _product;
        bidStack = _bidStack;
        offerStack = _offerStack;
    }
	// Get the product
	const T& GetProduct() const{
        return product;
    }
	// Get the bid stack
	const vector<Order>& GetBidStack() const{
        return bidStack;
    }
	// Get the offer stack
	const vector<Order>& GetOfferStack() const{
        return offerStack;
    }
	// Get the best bid/offer order
    BidOffer GetBidOffer() const {
        double highestBidPrice = std::numeric_limits<double>::lowest();
        Order bestBidOrder;
        //get the length of bidStack
        for (const auto& order : bidStack) {
            double price = order.GetPrice();
            if (price > highestBidPrice) {
                highestBidPrice = price;
                bestBidOrder = order;
            }
        }

        double lowestOfferPrice = std::numeric_limits<double>::max();
        Order bestOfferOrder;
        for (const auto& order : offerStack) {
            double price = order.GetPrice();
            if (price < lowestOfferPrice) {
                lowestOfferPrice = price;
                bestOfferOrder = order;
            }
        }
//        cout << "best bid price: " << bestBidOrder.GetPrice() << endl;
//        cout << "best offer price: " << bestOfferOrder.GetPrice() << endl;
        return BidOffer(bestBidOrder, bestOfferOrder);
    }
private:
	T product;
	vector<Order> bidStack;
	vector<Order> offerStack;
};


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class MarketDataConnector;

/**
* Market Data Service which distributes market data
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class MarketDataService : public Service<string, OrderBook<T>>{
private:
	map<string, OrderBook<T>> PidOrderBooksMap; //product_id -----> orderbook
	vector<ServiceListener<OrderBook<T>>*> listeners;
	MarketDataConnector<T>* connector;
	int bookDepth;
public:
	// Constructor and destructor
	MarketDataService(){
        PidOrderBooksMap = map<string, OrderBook<T>>();
        listeners = vector<ServiceListener<OrderBook<T>>*>();
        connector = new MarketDataConnector<T>(this);
        bookDepth = 5;
    }
	~MarketDataService() = default;
	// Get data on our service given a key
	OrderBook<T>& GetData(string _key){
        return PidOrderBooksMap[_key];
    }
	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(OrderBook<T>& _data){
        PidOrderBooksMap[_data.GetProduct().GetProductId()] = _data;
        for (auto& l : listeners){
            l->ProcessAdd(_data);
        }
    }
	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<OrderBook<T>>* _listener){
        listeners.push_back(_listener);
    }
	// Get all listeners on the Service
	const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const{
        return listeners;
    }
	// Get the connector of the service
	MarketDataConnector<T>* GetConnector(){
        return connector;
    }
	// Get the order book depth of the service
	int GetBookDepth() const{
        return bookDepth;
    }
	// Get the best bid/offer order
	const BidOffer& GetBestBidOffer(const string& _productId){
        return PidOrderBooksMap[_productId].GetBidOffer();
    }

    const OrderBook<Bond>& AggregateDepth(const string& _productId) {
        OrderBook<T>& orderBook = PidOrderBooksMap[_productId];

        // Aggregate bid stack
        vector<Order>& bidStack = orderBook.GetBidStack();
        unordered_map<double, long> bidAggMap;
        for (auto& order : bidStack) {
            double price = order.GetPrice();
            bidAggMap[price] += order.GetQuantity();
        }
        vector<Order> aggregatedBids;
        for (auto& item : bidAggMap) {
            aggregatedBids.push_back(Order(item.first, item.second, BID));
        }

        // Aggregate offer stack
        vector<Order>& offerStack = orderBook.GetOfferStack();
        unordered_map<double, long> offerAggMap;
        for (auto& order : offerStack) {
            double price = order.GetPrice();
            offerAggMap[price] += order.GetQuantity();
        }
        vector<Order> aggregatedOffers;
        for (auto& item : offerAggMap) {
            aggregatedOffers.push_back(Order(item.first, item.second, OFFER));
        }

        // Update and return the aggregated order book
        orderBook = OrderBook<T>(orderBook.GetProduct(), aggregatedBids, aggregatedOffers);
        return orderBook;
    }
};
/**
* Market Data Connector subscribing data to Market Data Service.
* Type T is the product type.
*/
template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>{
private:
	MarketDataService<T>* service;
public:
	MarketDataConnector(MarketDataService<T>* _service){ // Connector and Destructor
        service = _service;
    }
	~MarketDataConnector() = default;
	void Publish(OrderBook<T>& _data){ // Publish data to the Connector
        service->OnMessage(_data);
    }
    void Subscribe(std::ifstream& dataStream) {
        const int bookDepth = service->GetBookDepth();
        const int threadCount = bookDepth * 2;
        long count = 0;
        std::vector<Order> bidStack, offerStack;
        std::string line;
        while (std::getline(dataStream, line)) {
            std::stringstream lineStream(line);
            std::string productId, cell;
            double price;
            long quantity;
            PricingSide side;

            std::getline(lineStream, productId, ',');
            std::getline(lineStream, cell, ',');
            price = ConvertPrice(cell);
            std::getline(lineStream, cell, ',');
            quantity = std::stol(cell);
            std::getline(lineStream, cell, ',');
            side = (cell == "BID") ? BID : OFFER;
            Order order(price, quantity, side);
            (side == BID ? bidStack : offerStack).push_back(order);

//            // output the size of bidStack and offerStack
//            cout << "bidStack size: " << bidStack.size() << endl;
//            cout << "offerStack size: " << offerStack.size() << endl;
            count++;
            if (count % threadCount == 0) {
                T product = GetBond(productId);
                OrderBook<T> orderBook(product, bidStack, offerStack);
                service->OnMessage(orderBook);
            }
        }
    }
};


#endif
