/**
* streamingservice.hpp
* Defines the data types and Service for price streams.
* Data should flow via ServiceListener from the BondAlgoStreamingService.
* The BondStreamingService should use a Connector to publish streams via socket into a separate process which listens to the streams on the socket via its own Connector and prints them when it receives them.
* @author Breman Thuraisingham
*/

#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "algostreamingservice.hpp"

template<typename T>
class StreamingListenerFromAlgoStreaming;
template<typename T>
class StreamingServiceConnector;

/**
* Streaming service to publish two-way prices.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class StreamingService : public Service<string, PriceStream<T>> {
private:
    map<string, PriceStream<T>> priceStreams;
    vector<ServiceListener<PriceStream<T>>*> listeners;
    ServiceListener<AlgoStream<T>>* listener;
    StreamingServiceConnector<T>* connector;

public:
    StreamingService() {
        priceStreams = map<string, PriceStream<T>>();
        listeners = vector<ServiceListener<PriceStream<T>>*>();
        listener = new StreamingListenerFromAlgoStreaming<T>(this);
        connector = new StreamingServiceConnector<T>(this);
    }
    ~StreamingService() = default;
    // Get data on our service given a key
    PriceStream<T>& GetData(string _key) {
        return priceStreams[_key];
    }
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(PriceStream<T>& _data) {
        priceStreams[_data.GetProduct().GetProductId()] = _data;
    }
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<PriceStream<T>>* _listener) {
        listeners.push_back(_listener);
    }
    // Get all listeners on the Service
    const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const {
        return listeners;
    }
    // Get the listener of the service
    ServiceListener<AlgoStream<T>>* GetListener() {
        return listener;
    }
    // Publish two-way prices
    void PublishPrice(PriceStream<T>& _priceStream) {
        connector->Publish(_priceStream);
        for (auto& l : listeners) {
            l->ProcessAdd(_priceStream);
        }
    }
};

template<typename T>
class StreamingServiceConnector : public Connector<PriceStream<T>>
{
private:
    StreamingService<T>* service;
public:
    // Constructor
    StreamingServiceConnector(StreamingService<T>* _service) : service(_service) {}
    // Destructor
    ~StreamingServiceConnector() = default;
    // Publish data to the Connector
    void Publish(PriceStream<T>& data)override
    {
        // Print the price stream data
        T product = data.GetProduct();
        string productId = product.GetProductId();
        PriceStreamOrder bid = data.GetBidOrder();
        PriceStreamOrder offer = data.GetOfferOrder();

        cout << "Price Stream " << "(Product " << productId << "): \n"
             << "\tBid\t" << "Price: " << bid.GetPrice() << "\tVisibleQuantity: " << bid.GetVisibleQuantity()
             << "\tHiddenQuantity: " << bid.GetHiddenQuantity() << "\n"
             << "\tAsk\t" << "Price: " << offer.GetPrice() << "\tVisibleQuantity: " << offer.GetVisibleQuantity()
             << "\tHiddenQuantity: " << offer.GetHiddenQuantity() << "\n";
    }
    void Subscribe(ifstream& _data) override {}
};


/**
* Streaming Service Listener subscribing data from Algo Streaming Service to Streaming Service.
* Type T is the product type.
*/
template<typename T>
class StreamingListenerFromAlgoStreaming : public ServiceListener<AlgoStream<T>> {
private:
    StreamingService<T>* service;
public:
    StreamingListenerFromAlgoStreaming(StreamingService<T>* _service) {
        service = _service;
    }
    ~StreamingListenerFromAlgoStreaming() = default;
    // Listener callback to process an add event to the Service
    void ProcessAdd(AlgoStream<T>& _data) {
        PriceStream<T>* _priceStream = _data.GetPriceStream();
        service->OnMessage(*_priceStream);
        service->PublishPrice(*_priceStream);
    }
    // Listener callback to process a remove event to the Service
    void ProcessRemove(AlgoStream<T>& _data) {
    }
    // Listener callback to process an update event to the Service
    void ProcessUpdate(AlgoStream<T>& _data) {
    }
};

#endif
