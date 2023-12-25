/**
* guiservice.hpp
* Defines the data types and Service for GUI output.
*
* @author Breman Thuraisingham
*/
#ifndef GUI_SERVICE_HPP
#define GUI_SERVICE_HPP

#include "soa.hpp"
#include "pricingservice.hpp"

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class GUIConnector;
template<typename T>
class GUIToPricingListener;

/**
* Service for outputing GUI with a certain throttle.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class GUIService : Service<string, Price<T>>
{
private:
    map<string, Price<T>> guis;
    vector<ServiceListener<Price<T>>*> listeners;
    GUIConnector<T>* connector;
    ServiceListener<Price<T>>* listener;
    int throttle;
    long millisec;

public:
    // Constructor
    GUIService() : throttle(300), millisec(0)
    {
        guis = map<string, Price<T>>();
        listeners = vector<ServiceListener<Price<T>>*>();
        connector = new GUIConnector<T>(this);
        listener = new GUIToPricingListener<T>(this);
    }

    // Destructor
    ~GUIService() {}

    // Get data on our service given a key
    Price<T>& GetData(string _key)
    {
        return guis[_key];
    }

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T>& _data)
    {
        guis[_data.GetProduct().GetProductId()] = _data;
        connector->Publish(_data);
    }

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Price<T>>* _listener)
    {
        listeners.push_back(_listener);
    }

    // Get all listeners on the Service
    const vector<ServiceListener<Price<T>>*>& GetListeners() const
    {
        return listeners;
    }

    // Get the connector of the service
    GUIConnector<T>* GetConnector()
    {
        return connector;
    }

    // Get the listener of the service
    ServiceListener<Price<T>>* GetListener()
    {
        return listener;
    }

    // Get the throttle of the service
    int GetThrottle() const
    {
        return throttle;
    }

    // Get the millisec of the service
    long GetMillisec() const
    {
        return millisec;
    }

    // Set the millisec of the service
    void SetMillisec(long _millisec)
    {
        millisec = _millisec;
    }
};
template<typename T>
class GUIConnector : public Connector<Price<T>>
{
private:
    GUIService<T>* service;

public:
    // Constructor
    GUIConnector(GUIService<T>* _service) : service(_service) {}

    // Destructor
    ~GUIConnector() {}

    // Publish data to the Connector
    void Publish(Price<T>& _data)
    {
        int _throttle = service->GetThrottle();
        long _millisec = service->GetMillisec();
        long _millisecNow = GetMillisecond();
        while (_millisecNow < _millisec) _millisecNow += 1000;
        if (_millisecNow - _millisec >= _throttle)
        {
            service->SetMillisec(_millisecNow);
            ofstream _file;
            _file.open("gui.txt", ios::app);

            _file << TimeStamp() << ",";
            vector<string> _strings = _data.ToStrings();
            for (auto& s : _strings)
            {
                _file << s << ",";
            }
            _file << endl;
        }
    }

    // Subscribe data from the Connector
    void Subscribe(ifstream& _data){}
};

/**
* GUI Service Listener subscribing data to GUI Data.
* Type T is the product type.
*/
template<typename T>
class GUIToPricingListener : public ServiceListener<Price<T>>
{
private:
    GUIService<T>* service;

public:
    // Constructor
    GUIToPricingListener(GUIService<T>* _service) : service(_service) {}

    // Destructor
    ~GUIToPricingListener() {}

    // Listener callback to process an add event to the Service
    void ProcessAdd(Price<T>& _data)
    {
        service->OnMessage(_data);
    }

    // Listener callback to process a remove event to the Service
    void ProcessRemove(Price<T>& _data)
    {}

    // Listener callback to process an update event to the Service
    void ProcessUpdate(Price<T>& _data)
    {}
};

#endif
