#include "coreconf_handlers.h"
#include <coap3/coap.h>
#include <cxxopts.hpp>
#include <iostream>
#include <csignal>
#include <cstring>

// Global flag for graceful shutdown
static volatile bool running = true;

// Signal handler for clean shutdown
void signal_handler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    running = false;
}

int main(int argc, char *argv[]) {
    coap_context_t *ctx = nullptr;
    coap_endpoint_t *endpoint = nullptr;
    coap_address_t addr;
    int result = 0;

    // Parse command-line arguments
    cxxopts::Options options("ccserver", "CORECONF CoAP Server");
    options.add_options()
        ("h,host", "Bind address (hostname or IP)", cxxopts::value<std::string>()->default_value("::"))
        ("p,port", "Port number", cxxopts::value<uint16_t>()->default_value("5683"))
        ("help", "Print help message");

    std::string bind_address;
    uint16_t port;

    try {
        auto result_args = options.parse(argc, argv);

        if (result_args.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        bind_address = result_args["host"].as<std::string>();
        port = result_args["port"].as<uint16_t>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        std::cerr << options.help() << std::endl;
        return 1;
    }

    std::cout << "=== CORECONF Server ===" << std::endl;
    std::cout << "Starting CoAP server on " << bind_address << ":" << port << std::endl;

    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize CoAP library
    coap_startup();

    // Set log level (uses syslog levels: LOG_EMERG=0, LOG_INFO=6, LOG_DEBUG=7)
    coap_set_log_level(LOG_INFO);

    // Initialize address structure
    coap_address_init(&addr);
    addr.addr.sin6.sin6_family = AF_INET6;
    addr.addr.sin6.sin6_port = htons(port);
    addr.addr.sin6.sin6_addr = in6addr_any;

    // Create CoAP context
    ctx = coap_new_context(nullptr);
    if (!ctx) {
        std::cerr << "Failed to create CoAP context" << std::endl;
        result = 1;
        goto cleanup;
    }

    // Create endpoint
    endpoint = coap_new_endpoint(ctx, &addr, COAP_PROTO_UDP);
    if (!endpoint) {
        std::cerr << "Failed to create CoAP endpoint" << std::endl;
        result = 1;
        goto cleanup;
    }

    std::cout << "CoAP endpoint created successfully" << std::endl;

    // TODO: Load YANG models and SID mappings
    // This would involve:
    // 1. Loading .yang files or compiled schema
    // 2. Loading .sid files for SID to schema node mapping
    // 3. Initializing the data tree/datastore
    std::cout << "TODO: Load YANG models and initialize datastore" << std::endl;

    // Initialize CORECONF request handlers
    coreconf::initialize_handlers(ctx);

    std::cout << "Server ready, waiting for requests..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Main event loop
    while (running) {
        // Process CoAP events with 1 second timeout
        int events = coap_io_process(ctx, 1000);

        if (events < 0) {
            std::cerr << "Error in CoAP event processing" << std::endl;
            break;
        }

        // TODO: Add periodic tasks here if needed
        // - Check for datastore updates
        // - Send notifications
        // - Handle subscriptions (if implementing observe/notify)
    }

    std::cout << "Server shutting down..." << std::endl;

cleanup:
    // Clean up CORECONF resources
    coreconf::cleanup_handlers();

    // Clean up CoAP resources
    if (ctx) {
        coap_free_context(ctx);
    }

    coap_cleanup();

    std::cout << "Server stopped" << std::endl;
    return result;
}
