# README for Bond Trading System Project

## Project Overview

This project involves the development of a sophisticated bond trading system for US Treasuries. It includes an extensive set of services and components to handle various aspects of bond trading, such as trade booking, pricing, market data, risk management, and more. The system is designed to handle seven types of US Treasury securities: 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y.

## Key Components

The project is built around a core framework defined in `soa.hpp`, introducing the concepts of `Service`, `ServiceListener`, and `Connector`:

- **Service**: A central component that manages specific types of data (e.g., pricing, trade booking).
- **ServiceListener**: Listens to events on a service, such as data addition, updates, or removal.
- **Connector**: Bridges external data sources (files, sockets, etc.) to the services, using `OnMessage()` for data flow and `Publish()` for sending data back to external sources.

## Data Flow

Certain services will interact with external data through files and socket communication:
<img width="977" alt="image" src="https://github.com/PlumloLee/9815-Bond-Trading-System/assets/73246048/30719176-76ac-4eb9-8769-8a31d9e7682e">

## Bond Specific Classes

### `pricingservice.hpp`

- **Purpose**: The `pricingservice.hpp` file is dedicated to defining data types and services related to internal pricing within the bond trading system. It focuses on managing and providing access to price data for various financial products.
- **Key Components**:
  - **`Price` Template Class**: Represents a price object, which includes mid-price and bid/offer spread. It is templated to handle different types of products. This class provides methods to access product details, mid-price, and the bid/offer spread.
  - **Integration with Other Services**: The inclusion of headers like `soa.hpp` and `products.hpp` suggests that this service interacts closely with the core architecture of the system (SOA) and various financial products.

### `tradebookingservice.hpp`

- **Purpose**: Defines the data types and service for trade booking within the bond trading system. It focuses on managing trade information, including price, side, quantity, and associated book for various financial products.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Adheres to the `Service` base class structure from `soa.hpp`, customizing its functionalities to manage trade data.
  - **Connector for Data Flow**: Uses a connector to read data from `trades.txt` and publish it into the trading system. This integration showcases the use of connectors for external data communication.
  - **Interaction with Position Service**: The service is expected to communicate with the `BondPositionService`, likely through a `ServiceListener`, demonstrating an event-driven approach within the SOA framework.
- **Key Components**:
  - **`Trade` Template Class**: Represents a trade with attributes such as product, trade ID, price, book, quantity, and side (BUY or SELL). This class is tailored to handle different product types.
  - **External Data Source Integration**: Highlights the process of reading trade data from an external file (`trades.txt`) and feeding it into the system, emphasizing the service's role in data ingestion and processing.

### `marketdataservice.hpp`

- **Purpose**: This header file is tailored for managing market data, particularly focusing on order book data for the bond trading system. It includes the definition of market data orders, including details like price, quantity, and side (bid or offer).
- **Key Components**:
  - **`PricingSide` Enumeration**: Distinguishes between the bid and offer sides in market data.
  - **`Order` Class**: Represents a market data order. It contains information such as price, quantity, and side (either BID or OFFER). This class includes methods to access these details.
  - **`BidOffer` Class**: A class representing a bid and offer order, likely including details to encapsulate and manage both sides of a market order.

### `guiservice.hpp`

- **Purpose**: The `guiservice.hpp` file defines the structures and services for GUI (Graphical User Interface) output in the bond trading system. It is designed to manage and display GUI elements, keyed on product identifiers.
- **Key Components**:
  - **`GUIService` Template Class**: A service class designed for GUI output. It inherits from the `Service` base class defined in `soa.hpp` and is templated to handle different product types. The class maintains a map of GUI elements keyed by product identifiers, a list of listeners, and connections for GUI-related data.
  - **Throttling Mechanism**: The service includes a throttling mechanism to control the frequency of GUI updates, essential for real-time display without overwhelming system resources.
  - **Connectors and Listeners**: The file mentions `GUIConnector` and `GUIToPricingListener`, suggesting integration with other services like the pricing service. These components are likely responsible for fetching and updating data relevant to the GUI.

### `positionservice.hpp`

- **Purpose**: Defines data types and service for managing positions in the bond trading system. It handles position information for various financial products across different trading books.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Follows the structure of the `Service` base class from `soa.hpp`, specifically tailored for managing position data.
  - **ServiceListener Integration**: The `BondPositionService` receives data via `ServiceListener` from the `BondTradeBookingService`, highlighting an event-driven approach. It should also link to the `BondRiskService` through a `ServiceListener`, emphasizing the SOA principle of decoupled services interacting through listeners.
- **Key Components**:
  - **`Position` Template Class**: Represents a position in a specific book for a given product type. It includes details of positions across different trading books.
  - **No Direct Connector Requirement**: The service does not use a connector for external data flow, relying instead on internal system communication through service listeners.

### `riskservice.hpp`

- **Purpose**: Defines the data types and service for managing fixed income risk in the bond trading system. It is primarily focused on handling PV01 (Price Value of a Basis Point) risk.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Follows the `Service` base class from `soa.hpp`, specifically adapted for managing risk-related data, particularly PV01 values.
  - **ServiceListener Integration**: Explicitly mentioned that `BondRiskService` should be linked to `BondPositionService` via a `ServiceListener`. This underscores the SOA principle where services interact through listeners rather than direct references, enhancing modularity and decoupling.
- **Key Components**:
  - **`PV01` Template Class**: Represents the PV01 risk associated with a particular product. Includes product details, PV01 value, and quantity.
  - **Risk Calculation and Management**: The service likely involves calculating and updating PV01 values for various positions, emphasizing its role in risk assessment and management.

### `inquiryservice.hpp`

- **Purpose**: This header file focuses on handling customer inquiries within the bond trading system. It outlines the structure and behavior of services related to inquiries, defining how they are received, processed, and responded to.
- **Key Components**:
  - **`InquiryState` Enumeration**: Defines various states of an inquiry, such as `RECEIVED`, `QUOTED`, `DONE`, `REJECTED`, and `CUSTOMER_REJECTED`.
  - **`Inquiry` Template Class**: Represents a customer inquiry with attributes like inquiry ID, product, side (buy or sell), quantity, price, and current state.
  - **Service Flow**: The file describes a service that reads inquiries from `inquiries.txt`. Each inquiry starts with a state of `RECEIVED`. The service processes these inquiries by sending quotes, updating their states, and notifying the connector. The connector then publishes the updated inquiries back to the service

### `streamingservice.hpp`

- **Purpose**: Defines the data types and service for price streams in the bond trading system. It focuses on the management and dissemination of two-way price streams, keyed on product identifiers.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Inherits from the `Service` base class defined in `soa.hpp`. It overrides the core functionalities to specifically handle `PriceStream` objects.
  - **Interconnection**: Data flows into this service via `ServiceListener` from the `BondAlgoStreamingService`. This indicates a listener-based integration where the `StreamingService` is a consumer of data produced by `BondAlgoStreamingService`.
  - **`StreamingServiceConnector`**: Uses a connector to publish streams via sockets into a separate process. This demonstrates the use of the connector concept to facilitate external communication.
- **Key Components**:
  - **`StreamingService` Template Class**: Manages two-way price streams, with methods for adding, updating, and removing price stream data.
  - **Listeners and Connectors**: Incorporates listeners (`StreamingListenerFromAlgoStreaming`) and a connector (`StreamingServiceConnector`) for interacting with other parts of the system, demonstrating the SOA approach.

### `algostreamingservice.hpp`

- **Purpose**: This header file is focused on algorithmic streaming services in the trading system. It includes the definition and handling of price stream orders, crucial for algorithmic trading strategies.
- **Key Components**:
  - **`PriceStreamOrder` Class**: Represents a price stream order, which is a key element in algorithmic trading. It contains details such as price, visible and hidden quantities, and the side of the order (PricingSide). This class provides methods to access these attributes.
  - **Integration with Pricing Service**: The file likely interacts with the pricing service (`pricingservice.hpp`), as indicated by the inclusion of this header, to utilize real-time pricing data for algorithmic streaming.

### `executionservice.hpp`

- **Purpose**: This file defines the data types and service for executing orders on an exchange within the bond trading system. It is focused on managing execution orders, keyed on product identifiers.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Inherits from the `Service` base class defined in `soa.hpp`, tailoring its functionalities to handle `ExecutionOrder` objects.
  - **Connectivity and Listeners**: Utilizes `ExecutionServiceConnector` for external connectivity, indicating its role in communicating execution orders beyond the immediate system. It also incorporates `ExecutionToAlgoExecutionListener`, suggesting integration with algorithmic execution services.
- **Key Components**:
  - **`ExecutionService` Template Class**: Manages execution orders, including methods to add, update, and remove data. It maintains a map of execution orders and a list of listeners.
  - **Listeners and Connector Integration**: The presence of listeners and a connector underlines the system's SOA approach, showing how this service communicates with other parts of the system.

### `algoexecutionservice.hpp`

- **Purpose**: This header file focuses on the aspect of algorithmic executions within the trading system. It provides the necessary structures and definitions for creating and managing execution orders that can be placed on exchanges.
- **Key Components**:
  - **`OrderType` Enumeration**: Defines various types of orders such as `FOK` (Fill or Kill), `IOC` (Immediate or Cancel), `MARKET`, `LIMIT`, and `STOP`.
  - **`Market` Enumeration**: Enumerates different markets like `BROKERTEC`, `ESPEED`, and `CME`.
  - **`ExecutionOrder` Template Class**: Represents an execution order with attributes including product, side, order ID, order type, price, visible and hidden quantities, parent order ID, and a flag indicating if it is a child order. The class is templated to be applicable to different product types.

### `historicaldataservice.hpp`

- **Purpose**: Defines the data types and service for processing and persisting historical data in the bond trading system. It deals with various types of historical data, which are essential for analysis, reporting, and compliance.
- **Integration with Core Architecture**:
  - **Inherits `Service` Base Class**: Inherits from the `Service` base class provided in `soa.hpp`, specializing in handling historical data of various types.
  - **Connector and Listener Integration**: Utilizes `HistoricalDataConnector` for potentially exporting data to external storage or systems. It also uses `HistoricalDataListener` to interact with other parts of the system, aligning with the SOA pattern of decoupled components communicating through listeners.
- **Key Components**:
  - **`HistoricalDataService` Template Class**: Manages historical data keyed on a persistent key. It includes functionalities to add, update, remove, and retrieve historical data.
  - **`ServiceType` Enumeration**: Identifies different types of services like POSITION, RISK, EXECUTION, STREAMING, and INQUIRY, suggesting the diverse range of data this service handles.




## File Format and Notation

- US Treasuries prices use fractional notation (e.g., 100-xyz).
- Output files include timestamps with millisecond precision.

## Dependencies

- Ensure C++14 or later is used for compatibility.
- External libraries or frameworks used (if any) should be listed here.
  - Boost

## Building and Running

- Instructions on how to build and run the system, including any necessary configuration steps.

## Sample Data Creation

- Guidelines on generating sample data for `prices.txt` and `trades.txt`.

## Contribution

- Guidelines for contributing to the project, including coding standards and pull request procedures.

---

Note: This README provides a high-level overview and should be expanded with detailed instructions and descriptions tailored to the specifics of the system and its components.
