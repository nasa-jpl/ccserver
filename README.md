# CORECONF CoAP Server

A C++ implementation of a CORECONF (YANG-CBOR) server using libcoap and nanocbor.

## Overview

This server implements the CORECONF protocol for managing network devices using YANG data models over CoAP with CBOR encoding. It provides placeholder handlers for the three main CORECONF operations:

- **GET**: Retrieve a single data node
- **FETCH**: Retrieve multiple data nodes in a single request
- **iPATCH**: Apply partial updates to configuration data

## Dependencies

- **libcoap-3**: CoAP protocol implementation
- **nanocbor**: Lightweight CBOR encoder/decoder
- **cxxopts**: Command-line argument parsing (included as submodule)

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get install libcoap3-dev
```

For nanocbor, you may need to build from source:
```bash
git clone https://github.com/bergzand/NanoCBOR.git
cd NanoCBOR
mkdir build && cd build
cmake ..
make
sudo make install
```

## Building

```bash
# Clone with submodules
git clone --recursive <repository-url>

# Or if already cloned, initialize submodules
git submodule update --init

# Build
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
# Run with default settings (bind to all interfaces on port 5683)
./ccserver

# Specify host and port
./ccserver --host 127.0.0.1 --port 5684

# Show help
./ccserver --help
```

## Project Structure

```
ccserver/
├── CMakeLists.txt         # Build configuration
├── README.md              # This file
├── external/              # Third-party dependencies
│   └── cxxopts/          # Command-line parsing (submodule)
├── include/               # Header files
│   └── coreconf_handlers.h
└── src/                   # Source files
    ├── main.cpp           # Server initialization and main loop
    └── coreconf_handlers.cpp  # Request handlers
```

## Implementation Status

### Current Features
- ✅ CoAP server initialization
- ✅ Command-line argument parsing
- ✅ Basic request routing for GET, FETCH, iPATCH
- ✅ CBOR payload parsing/encoding stubs

### TODO: YANG Model Integration
The current implementation provides placeholder handlers. To make this fully functional, you need to integrate YANG models:

1. **Load YANG Models**: Parse .yang files and .sid files to build the schema tree
2. **SID Mapping**: Map YANG Schema Item iDentifiers (SIDs) to data nodes
3. **Datastore**: Implement a data tree to store configuration and state data
4. **Validation**: Validate requests against YANG constraints (type, range, must statements, etc.)
5. **Data Access**: Implement actual data retrieval and modification based on SIDs

### Request Handler TODOs

#### GET Handler (`handle_get`)
- Parse URI path to extract SID or data node path
- Look up the data node in the YANG schema
- Retrieve current value from datastore
- Encode response as CBOR

#### FETCH Handler (`handle_fetch`)
- Parse CBOR array of SIDs from request payload
- For each SID:
  - Validate against YANG model
  - Retrieve data node value
  - Handle subtree retrieval if needed
- Construct CBOR map response (SID → value)

#### iPATCH Handler (`handle_ipatch`)
- Parse CBOR map of SID → value pairs
- Validate each update:
  - SID exists in model
  - Value matches YANG type constraints
  - Access permissions (if applicable)
- Apply updates atomically (all or nothing)
- Handle list operations (create, delete, reorder)
- Trigger notifications if needed

## CORECONF Resources

- [RFC 8132](https://www.rfc-editor.org/rfc/rfc8132.html) - PATCH and FETCH Methods for CoAP
- [RFC 9254](https://www.rfc-editor.org/rfc/rfc9254.html) - YANG-CBOR encoding
- [draft-ietf-core-comi](https://datatracker.ietf.org/doc/draft-ietf-core-comi/) - CoAP Management Interface

## License

[Specify your license here]
