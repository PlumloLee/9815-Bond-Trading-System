/**
* executionservice.hpp
* Defines the data types and Service for executions.
*
* @author Breman Thuraisingham
*/
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "algoexecutionservice.hpp"

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class ExecutionToAlgoExecutionListener;
template<typename T>
class ExecutionServiceConnector;

/**
* Service for executing orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder<T>>
{
private:
    map<string, ExecutionOrder<T>> executionOrders;
    ExecutionServiceConnector<T>* connector; // connector related to this server
    vector<ServiceListener<ExecutionOrder<T>>*> listeners;
    ExecutionToAlgoExecutionListener<T>* listener;

public:
    // Constructor
    ExecutionService()
    {
        executionOrders = map<string, ExecutionOrder<T>>();
        listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
        listener = new ExecutionToAlgoExecutionListener<T>(this);
        connector = new ExecutionServiceConnector<T>(this);
    }
    // Destructor
    ~ExecutionService() {}
    // Get data on our service given a key
    ExecutionOrder<T>& GetData(string _key)
    {
        return executionOrders[_key];
    }
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(ExecutionOrder<T>& _data)
    {
        executionOrders[_data.GetProduct().GetProductId()] = _data;
    }
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<ExecutionOrder<T>>* _listener)
    {
        listeners.push_back(_listener);
    }
    // Get all listeners on the Service
    const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const
    {
        return listeners;
    }
    // Get the listener of the service
    ExecutionToAlgoExecutionListener<T>* GetListener()
    {
        return listener;
    }
    // Execute an order on a market
    void ExecuteOrder(ExecutionOrder<T>& _executionOrder)
    {
        string _productId = _executionOrder.GetProduct().GetProductId();
        executionOrders[_productId] = _executionOrder;
        connector->Publish(_executionOrder);
        for (auto& l : listeners)
        {
            l->ProcessAdd(_executionOrder);
        }

    }
};


template<typename T>
class ExecutionServiceConnector : public Connector<ExecutionOrder<T>>
{
private:
    ExecutionService<T>* service; // Execution service related to this connector

public:
    // Constructor
    ExecutionServiceConnector(ExecutionService<T>* _service) : service(_service) {}
    // Destructor
    ~ExecutionServiceConnector() = default;
    // Publish data to the Connector
    void Publish(ExecutionOrder<T>& order) override{
        // print the execution order data
        auto product = order.GetProduct();
        string order_type;
        switch (order.GetOrderType()) {
            case FOK: order_type = "FOK"; break;
            case MARKET: order_type = "MARKET"; break;
            case LIMIT: order_type = "LIMIT"; break;
            case STOP: order_type = "STOP"; break;
            case IOC: order_type = "IOC"; break;
        }

        cout << "ExecutionOrder: \n"
             << "\tProduct: " << product.GetProductId() << "\tOrderId: " << order.GetOrderId()  << "\n"
             << "\tPricingSide: " << (order.GetPricingSide() == BID ? "Bid" : "Offer")
             << "\tOrderType: " << order_type << "\t\tIsChildOrder: " << (order.IsChildOrder() ? "True" : "False") << "\n"
             << "\tPrice: " << order.GetPrice() << "\tVisibleQuantity: " << order.GetVisibleQuantity()
             << "\tHiddenQuantity: " << order.GetHiddenQuantity() << endl << endl;
    }
    void Subscribe(ifstream& _data) override {}
};

/**
* Execution Service Listener subscribing data from Algo Execution Service to Execution Service.
* Type T is the product type.
*/
template<typename T>
class ExecutionToAlgoExecutionListener : public ServiceListener<AlgoExecution<T>>
{
private:
    ExecutionService<T>* service;
public:
    // Constructor
    ExecutionToAlgoExecutionListener(ExecutionService<T>* _service) : service(_service) {}
    // Destructor
    ~ExecutionToAlgoExecutionListener() {}
    // Listener callback to process an add event to the Service
    void ProcessAdd(AlgoExecution<T>& _data)
    {
        ExecutionOrder<T>* _executionOrder = _data.GetExecutionOrder();
        service->OnMessage(*_executionOrder);
        service->ExecuteOrder(*_executionOrder);
    }
    // Listener callback to process a remove event to the Service
    void ProcessRemove(AlgoExecution<T>& _data)
    {
        // Implementation for ProcessRemove
    }
    // Listener callback to process an update event to the Service
    void ProcessUpdate(AlgoExecution<T>& _data)
    {
        // Implementation for ProcessUpdate
    }
};

#endif
