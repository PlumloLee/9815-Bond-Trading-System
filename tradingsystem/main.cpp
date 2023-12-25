#include <iostream>
#include <map>

#include "soa.hpp"
#include "products.hpp"
#include "algoexecutionservice.hpp"
#include "algostreamingservice.hpp"
#include "executionservice.hpp"
#include "guiservice.hpp"
#include "historicaldataservice.hpp"
#include "inquiryservice.hpp"
#include "marketdataservice.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "streamingservice.hpp"
#include "tradebookingservice.hpp"

using namespace std;

int main()
{
    // 1. define data path and generate data
    log(LogLevel::INFO, "Generating price and orderbook data...");

    string prices_path = "../data/prices.txt";
    string trades_path = "../data/trades.txt";
    string marketdata_path = "../data/marketdata.txt";
    string inquiries_path = "../data/inquiries.txt";

    // 2. start trading service
    log(LogLevel::INFO, "Initializing trading service...");
	PricingService<Bond> pricingService;
	TradeBookingService<Bond> tradeBookingService;
	PositionService<Bond> positionService;
	RiskService<Bond> riskService;
	MarketDataService<Bond> marketDataService;
	AlgoExecutionService<Bond> algoExecutionService;
	AlgoStreamingService<Bond> algoStreamingService;
	GUIService<Bond> guiService;
	ExecutionService<Bond> executionService;
	StreamingService<Bond> streamingService;
	InquiryService<Bond> inquiryService;
	HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
	HistoricalDataService<PV01<Bond>> historicalRiskService(RISK);
	HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
	HistoricalDataService<PriceStream<Bond>> historicalStreamingService(STREAMING);
	HistoricalDataService<Inquiry<Bond>> historicalInquiryService(INQUIRY);
    log(LogLevel::INFO, "Trading service Initialized.");

    // 3. link services
    log(LogLevel::INFO, "Linking services...");
	pricingService.AddListener(algoStreamingService.GetListener());
	pricingService.AddListener(guiService.GetListener());
	algoStreamingService.AddListener(streamingService.GetListener());
	streamingService.AddListener(historicalStreamingService.GetListener());
	marketDataService.AddListener(algoExecutionService.GetListener());
	algoExecutionService.AddListener(executionService.GetListener());
	executionService.AddListener(tradeBookingService.GetListener());
	executionService.AddListener(historicalExecutionService.GetListener());
	tradeBookingService.AddListener(positionService.GetListener());
	positionService.AddListener(riskService.GetListener());
	positionService.AddListener(historicalPositionService.GetListener());
	riskService.AddListener(historicalRiskService.GetListener());
	inquiryService.AddListener(historicalInquiryService.GetListener());
    log(LogLevel::INFO, "Services linked.");

    // 4. start Price data service
    log(LogLevel::INFO, "Price data Retrieving .");
	ifstream priceData(prices_path);
	pricingService.GetConnector()->Subscribe(priceData);

	log(LogLevel::INFO, "Price data Retrieved.");

    // 5. start Trade data service
    log(LogLevel::INFO, "Trade data Retrieving .");
	ifstream tradeData(trades_path);
	tradeBookingService.GetConnector()->Subscribe(tradeData);
	log(LogLevel::INFO, "Trade data Retrieved.");

    // 6. start Market data service
    log(LogLevel::INFO, "Market data Retrieving .");
	ifstream marketData(marketdata_path);
	marketDataService.GetConnector()->Subscribe(marketData);
	log(LogLevel::INFO, "Market data Retrieved.");

    // 7. start Inquiry data service
    log(LogLevel::INFO, "Inquiry data Retrieving .");
	ifstream inquiryData(inquiries_path);
	inquiryService.GetConnector()->Subscribe(inquiryData);
    log(LogLevel::INFO, "Inquiry data Retrieved.");

	log(LogLevel::INFO, "Program Ended.");
	return 0;
}

