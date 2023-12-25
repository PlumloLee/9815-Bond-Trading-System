/**
* inquiryservice.hpp
* Defines the data types and Service for customer inquiries.
* read inquiries from a file called inquiries.txt with attributes for each inquiry with state of RECEIVED
 * For each inquiry, create an Inquiry object and pass it to the BondInquiryService.
 * The service, upon receiving an inquiry, sends a quote, updates the state, and notifies the connector.
 * The connector then publishes the inquiry back to the service with the updated state.
 *
* @author Breman Thuraisingham
*/
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>

enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

template<typename T>
class Inquiry {
public:
    // Constructor
    Inquiry() = default;
    Inquiry(const std::string& inquiryId, const T& product, Side side, long quantity, double price, InquiryState state)
        : inquiryId(inquiryId), product(product), side(side), quantity(quantity), price(price), state(state) {}
    // Getters and setters
    const std::string& GetInquiryId() const { return inquiryId; }
    const T& GetProduct() const { return product; }
    Side GetSide() const { return side; }
    long GetQuantity() const { return quantity; }
    double GetPrice() const { return price; }
    void SetPrice(double newPrice) { price = newPrice; }
    InquiryState GetState() const { return state; }
    void SetState(InquiryState newState) { state = newState; }
    std::vector<std::string> ToStrings() const {
        return { inquiryId, product.GetProductId(), SideToString(side), std::to_string(quantity), ConvertPrice(price), StateToString(state) };
    }

private:
    std::string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;
    static std::string SideToString(Side side) {
        switch (side) {
            case BUY: return "BUY";
            case SELL: return "SELL";
            default: return "";
        }
    }
    static std::string StateToString(InquiryState state) {
        switch (state) {
            case RECEIVED: return "RECEIVED";
            case QUOTED: return "QUOTED";
            case DONE: return "DONE";
            case REJECTED: return "REJECTED";
            case CUSTOMER_REJECTED: return "CUSTOMER_REJECTED";
            default: return "";
        }
    }
};


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class InquiryConnector;

/**
* Service for customer inquirry objects.
* Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
* Type T is the product type.
*/
template<typename T>
class InquiryService : public Service<string, Inquiry<T>>{
private:
	map<string, Inquiry<T>> inquiries;
	vector<ServiceListener<Inquiry<T>>*> listeners;
	InquiryConnector<T>* connector;
public:
	InquiryService(){
        // Constructor and destructor
        inquiries = map<string, Inquiry<T>>();
        listeners = vector<ServiceListener<Inquiry<T>>*>();
        connector = new InquiryConnector<T>(this);
    }
	~InquiryService() = default;

	Inquiry<T>& GetData(string _key) {
        // Get data on our service given a key
        return inquiries[_key];
    }
	void OnMessage(Inquiry<T>& _data){
        InquiryState _state = _data.GetState();
        switch (_state){
            case RECEIVED:
                // when the inquiry is in the RECEIVED state.
                // The BondInquiryService should send a quote of 100 back to a Connector via the Publish() method.
                inquiries[_data.GetInquiryId()] = _data;
                SendQuote(_data.GetInquiryId(), 100);
                connector->Publish(_data);
                break;
            case QUOTED:
                _data.SetState(DONE);
                inquiries[_data.GetInquiryId()] = _data;
                for (auto& l : listeners){
                    l->ProcessAdd(_data);
                }
                break;
            case DONE:
                inquiries[_data.GetInquiryId()] = _data;
                for (auto& l : listeners){
                    l->ProcessAdd(_data);
                }
                break;
            case REJECTED:
                // Handle REJECTED state if needed
                break;
            case CUSTOMER_REJECTED:
                // Handle CUSTOMER_REJECTED state if needed
                break;
            default:
                // Optional: Handle unexpected states
                break;
        }
    }
	void AddListener(ServiceListener<Inquiry<T>>* _listener){
        // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
        listeners.push_back(_listener);
    }
	const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const{
        // Get all listeners on the Service
        return listeners;
    }
	InquiryConnector<T>* GetConnector(){
        // Get the connector of the service
        return connector;
    }

	void SendQuote(const string& _inquiryId, double _price) {
        // Send a quote back to the client
        Inquiry<T> &_inquiry = inquiries[_inquiryId];
        if (_inquiry.GetState() == RECEIVED) {
            _inquiry.SetPrice(_price);
            for (auto &l: listeners) {
                l->ProcessAdd(_inquiry);
            }
        }
    }
	void RejectInquiry(const string& _inquiryId){
        // Reject an inquiry from the client
        Inquiry<T>& _inquiry = inquiries[_inquiryId];
        _inquiry.SetState(REJECTED);
    }
};

/**
* Inquiry Connector subscribing data to Inquiry Service and publishing data from Inquiry Service.
* Type T is the product type.
*/

template<typename T>
class InquiryListener : public ServiceListener<Inquiry<T>> {
    InquiryService<T>& service;
public:
    explicit InquiryListener(InquiryService<T>& svc) : service(svc) {}
    void ProcessAdd(Inquiry<T>& inquiry) override {
        if (inquiry.GetState() == RECEIVED) {
            service.SendQuote(inquiry.GetInquiryId(), 100);
        }
    }
};


template<typename T>
class InquiryConnector: public Connector<Inquiry<T>>{
private:
	InquiryService<T>* service;
    Side StringToSide(const std::string& str) {
        if (str == "BUY") return BUY;
        else return SELL;
    }
public:
	InquiryConnector(InquiryService<T>* _service){
        // Connector and Destructor
        service = _service;
    }
	~InquiryConnector() = default;

	void Publish(Inquiry<T>& _data){
        // The BondInquiryService should send a quote of 100 back to a Connector via the Publish() method.
        // The Connector should transition the inquiry to the QUOTED state
        // and send it back to the BondInquiryService via the OnMessage method with the supplied price.
        InquiryState _state = _data.GetState();
        if (_state == RECEIVED)
        {
            _data.SetState(QUOTED);
            service->OnMessage(_data);
            // It should then immediately send an update of the Inquiry object with a DONE state.
            _data.SetState(DONE);
            service->OnMessage(_data);
        }
    }
    // Inquiry Reading: Read inquiries from inquiries.txt and create Inquiry objects with the state RECEIVED.
    void Subscribe(std::ifstream& dataStream) {
        std::string line;
        while (std::getline(dataStream, line)) {
            std::stringstream lineStream(line);
            std::vector<std::string> cells;
            std::string cell;
            while (std::getline(lineStream, cell, ',')) {
                cells.push_back(cell);
            }

            if (cells.size() < 6) {
                // Handle error: insufficient data in line
                continue;
            }

            std::string inquiryId = cells[0];
            std::string productId = cells[1];
            Side side = StringToSide(cells[2]); // Assuming StringToSide is implemented
            long quantity = std::stol(cells[3]);
            double price = ConvertPrice(cells[4]);
            InquiryState state = RECEIVED; // Assuming StringToInquiryState is implemented
            T product = GetBond(productId);
            Inquiry<T> inquiry(inquiryId, product, side, quantity, price, state);
            service->OnMessage(inquiry);
        }
    }

};


#endif
