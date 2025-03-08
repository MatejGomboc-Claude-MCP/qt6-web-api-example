# Qt6 Web API Example

A simple Qt6 C++ web API that returns 'Hello World' and supports RFC 7807 Problem Details for HTTP error reporting.

## Features

- Simple HTTP API built with Qt6's HttpServer module
- Returns "Hello World" on the root endpoint
- Full implementation of RFC 7807 Problem Details for standardized error reporting
- JSON configuration file for easy deployment and customization
- Comprehensive security features:
  - Rate limiting to prevent abuse
  - TLS/HTTPS support
  - CORS support for web clients
  - Security headers (CSP, X-Frame-Options, etc.)
  - Exception safety with proper error handling
  - Configurable problem detail format
  - Local-only binding by default (for improved security)

## Requirements

- Qt 6.4 or higher (includes HttpServer module)
- C++17 compatible compiler
- CMake 3.18 or higher

## Building the Project

```bash
# Clone the repository
git clone https://github.com/MatejGomboc-Claude-MCP/qt6-web-api-example.git
cd qt6-web-api-example

# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
make
```

## Configuration

The API can be configured using a JSON configuration file (`config.json`) or command-line arguments. Command-line arguments take precedence over the configuration file.

### JSON Configuration

The default `config.json` file is organized into logical sections:

```json
{
  "server": {
    "port": 8080,
    "address": "localhost",
    "workers": 4
  },
  "security": {
    "rateLimit": {
      "enabled": true,
      "maxRequestsPerMinute": 100,
      "ipWhitelist": ["127.0.0.1", "::1"]
    },
    "cors": {
      "enabled": false,
      "allowedOrigins": ["*"],
      "allowedMethods": ["GET", "POST", "OPTIONS"],
      "allowedHeaders": ["Content-Type", "Authorization"],
      "maxAge": 86400
    },
    "tls": {
      "enabled": false,
      "certificatePath": "",
      "keyPath": "",
      "passphrase": ""
    },
    "headers": {
      "contentTypeOptions": "nosniff",
      "frameOptions": "DENY",
      "contentSecurityPolicy": "default-src 'self'",
      "hstsMaxAge": 31536000,
      "hstsIncludeSubdomains": true
    }
  },
  "problemDetails": {
    "baseUrl": "https://problemdetails.example.com/problems",
    "includeDebugInfo": false,
    "contactEmail": ""
  },
  "logging": {
    "level": "info",
    "file": "",
    "console": true,
    "includeTimestamp": true
  }
}
```

### Using a Custom Config File

You can specify a custom configuration file:

```bash
./qt6-web-api-example --config /path/to/custom-config.json
```

### Command-Line Overrides

Command-line arguments can override configuration file settings:

```bash
# Override the port
./qt6-web-api-example --port 3000

# Enable CORS
./qt6-web-api-example --cors true

# Set rate limiting
./qt6-web-api-example --max-requests 60

# Enable TLS/HTTPS
./qt6-web-api-example --tls true --cert /path/to/cert.pem --key /path/to/key.pem
```

## Running the Server

```bash
# Run with default configuration
./qt6-web-api-example

# Run with custom configuration file
./qt6-web-api-example --config /path/to/config.json

# Get help on all options
./qt6-web-api-example --help
```

## API Endpoints

- `GET /` - Returns "Hello World" as plain text
- `GET /api` - Returns `{"message": "Hello World"}` as JSON
- `GET /api/not-found` - Example that returns a 404 ProblemDetail response
- `GET /api/error` - Example that returns a 500 ProblemDetail response

## Problem Details Implementation

This project includes a full implementation of [RFC 7807 - Problem Details for HTTP APIs](https://tools.ietf.org/html/rfc7807), which provides a standardized format for returning error information from HTTP APIs.

Problem detail responses include:

- A `type` URI that identifies the error type
- A `title` that provides a short summary of the problem
- A `status` that contains the HTTP status code
- An optional `detail` that provides more detailed information
- An optional `instance` that refers to the specific occurrence of the problem
- Optional extension members for additional context

All error responses use the `application/problem+json` content type as specified in the RFC.

## Security Features

### Rate Limiting

The API includes IP-based rate limiting to prevent abuse. By default, each client IP is limited to 100 requests per minute. You can configure this in the JSON configuration:

```json
"rateLimit": {
  "enabled": true,
  "maxRequestsPerMinute": 100,
  "ipWhitelist": ["127.0.0.1", "::1"]
}
```

### TLS/HTTPS Support

For production use, enable TLS in the configuration:

```json
"tls": {
  "enabled": true,
  "certificatePath": "/path/to/cert.pem",
  "keyPath": "/path/to/key.pem",
  "passphrase": "optional-passphrase"
}
```

### CORS Support

Cross-Origin Resource Sharing (CORS) headers can be configured:

```json
"cors": {
  "enabled": true,
  "allowedOrigins": ["https://example.com", "https://app.example.com"],
  "allowedMethods": ["GET", "POST", "OPTIONS"],
  "allowedHeaders": ["Content-Type", "Authorization"],
  "maxAge": 86400
}
```

### Security Headers

The API automatically includes security headers for all responses, which can be customized:

```json
"headers": {
  "contentTypeOptions": "nosniff",
  "frameOptions": "DENY",
  "contentSecurityPolicy": "default-src 'self'",
  "hstsMaxAge": 31536000,
  "hstsIncludeSubdomains": true
}
```

### Exception Handling

All routes include comprehensive exception handling to ensure that unexpected errors are properly caught and returned as ProblemDetail responses rather than crashing the server.

## License

MIT