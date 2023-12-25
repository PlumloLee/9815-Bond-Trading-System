/**
* riskservice.hpp
* Defines the data types and Service for fixed income risk.
* The BondRiskService should be linked to a BondPositionService via a ServiceListener and send all positions to the BondRiskService via the AddPosition() method
* (note that the BondPositionService should not have an explicit reference to the BondRiskService though or versa – link them through a ServiceListener).
 *
* @author Breman Thuraisingham
*/
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"

/**
* PV01 risk.
* Type T is the product type.
*/
template<typename T>
class PV01{
public:
	// ctor for a PV01 value
	PV01() = default;
	PV01(const T& _product, double _pv01, long _quantity){
        product = _product;
        pv01 = _pv01;
        quantity = _quantity;
    }
	const T& GetProduct() const{
        return product;
    }
	double GetPV01() const{
        return pv01;
    }
	long GetQuantity() const{
        return quantity;
    }
	// Set the quantity that this risk value is associated with
	void SetQuantity(long _quantity){
        quantity = _quantity;
    }
	vector<string> ToStrings() const{
        vector<string> _strings;
        _strings.push_back(product.GetProductId());
        _strings.push_back(to_string(pv01));
        _strings.push_back(to_string(quantity));
        return _strings;
    }

private:
	T product;
	double pv01;
	long quantity;
};


/**
* A bucket sector to bucket a group of securities.
* We can then aggregate bucketed risk to this bucket.
* Type T is the product type.
*/
template<typename T>
class BucketedSector{
public:
	BucketedSector() = default;
	BucketedSector(const vector<T>& _products, string _name){
        products = _products;
        name = _name;
    }
	const vector<T>& GetProducts() const{
        return products;
    }
	const string& GetName() const{
        return name;
    }
private:
	vector<T> products;
	string name;
};

template<typename T>
class RiskListenerFromPosition;

/**
* Risk Service to vend out risk for a particular security and across a risk bucketed sector.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class RiskService : public Service<string, PV01<T>>{
private:
	map<string, PV01<T>> PidPv01Map;  // product id -> pv01 value
	vector<ServiceListener<PV01<T>>*> listeners;
	RiskListenerFromPosition<T>* listener;
public:
	// Constructor and destructor
	RiskService(){
        listener = new RiskListenerFromPosition<T>(this);
    }
	~RiskService() = default;
	PV01<T>& GetData(string _key){
        return PidPv01Map[_key];
    }
	void OnMessage(PV01<T>& _data){
        PidPv01Map[_data.GetProduct().GetProductId()] = _data;
        for (auto& l : listeners){
            l->ProcessUpdate(_data);
        }
    }
	void AddListener(ServiceListener<PV01<T>>* _listener){
        listeners.push_back(_listener);
    }
	const vector<ServiceListener<PV01<T>>*>& GetListeners() const{
        return listeners;
    }
	RiskListenerFromPosition<T>* GetListener(){
        return listener;
    }
	// Add a position that the service will risk
    void AddPosition(Position<T>& _position) {
        const T& _product = _position.GetProduct(); // Use reference
        const string& _productId = _product.GetProductId(); // Use reference if possible
        double _pv01Value = GetPV01Value(_productId);
        long _quantity = _position.GetAggregatePosition();

        // Utilize move semantics if applicable for PV01 constructor
        PV01<T> _pv01(_product, _pv01Value, _quantity);
        PidPv01Map[_productId] = std::move(_pv01); // Use move assignment if PV01 is movable
        OnMessage(PidPv01Map[_productId]);
    }

    // Get the bucketed risk for the bucket sector
	const PV01<BucketedSector<T>>& GetBucketedRisk(const BucketedSector<T>& _sector) const{
        BucketedSector<T> _product = _sector;
        double _pv01 = 0;
        long _quantity = 1;

        vector<T>& _products = _sector.GetProducts();
        for (auto& p : _products){
            string _pId = p.GetProductId();
            _pv01 += PidPv01Map[_pId].GetPV01() * PidPv01Map[_pId].GetQuantity();
        }

        return PV01<BucketedSector<T>>(_product, _pv01, _quantity);
    }

};


/**
* Risk Service Listener subscribing data from Position Service to Risk Service.
* Type T is the product type.
*/
template<typename T>
class RiskListenerFromPosition : public ServiceListener<Position<T>>{
private:
	RiskService<T>* service;
public:
	// Connector and Destructor
	RiskListenerFromPosition(RiskService<T>* _service){
        service = _service;
    }
	~RiskListenerFromPosition() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(Position<T>& _data){
        service->AddPosition(_data);
    }
	// Listener callback to process a remove event to the Service
	void ProcessRemove(Position<T>& _data){
        // Do nothing
    }
	// Listener callback to process an update event to the Service
	void ProcessUpdate(Position<T>& _data){
        // Do nothing
    }
};
#endif
