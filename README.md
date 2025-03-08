# Qt6 Web API Example

A simple Qt6 C++ web API that returns 'Hello World' and supports RFC 7807 Problem Details for HTTP error reporting.

## Features

- Simple HTTP API built with Qt6's HttpServer module
- Returns "Hello World" on the root endpoint
- Full implementation of RFC 7807 Problem Details for standardized error reporting
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

## Running the Server

```bash
# Run with default settings (port 8080, localhost only)
./qt6-web-api-example

# Run on a specific port
./qt6-web-api-example --port 3000

# Run on all network interfaces (public access)
./qt6-web-api-example --address 0.0.0.0

# Enable CORS support
./qt6-web-api-example --cors

# Enable CORS with specific origins
./qt6-web-api-example --cors --cors-origins="https://example.com,https://app.example.com"

# Set rate limiting (requests per minute per client IP)
./qt6-web-api-example --rate-limit 60

# Disable rate limiting
./qt6-web-api-example --rate-limit 0

# Enable TLS/HTTPS
./qt6-web-api-example --tls --cert=/path/to/cert.pem --key=/path/to/key.pem

# Custom base URL for problem details
./qt6-web-api-example --problem-base-url="https://api.myapp.com/problems"

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

The API includes IP-based rate limiting to prevent abuse. By default, each client IP is limited to 100 requests per minute. You can configure this with the `--rate-limit` option.

When rate limiting is triggered, the API returns a proper 429 Too Many Requests response with a ProblemDetail object that includes:
- A `Retry-After` header
- A `retryAfter` extension in the problem detail

### TLS/HTTPS Support

For production use, enable TLS with your own certificates:

```bash
./qt6-web-api-example --tls --cert=/path/to/cert.pem --key=/path/to/key.pem
```

### CORS Support

Cross-Origin Resource Sharing (CORS) headers can be enabled for web clients:

```bash
./qt6-web-api-example --cors
```

By default, this allows any origin (`*`). For production, specify the allowed origins:

```bash
./qt6-web-api-example --cors --cors-origins="https://example.com,https://app.example.com"
```

### Security Headers

The API automatically includes security headers for all responses:
- `X-Content-Type-Options: nosniff`
- `X-Frame-Options: DENY`
- `Content-Security-Policy: default-src 'self'`
- `Strict-Transport-Security: max-age=31536000; includeSubDomains` (when TLS is enabled)

### Exception Handling

All routes include comprehensive exception handling to ensure that unexpected errors are properly caught and returned as ProblemDetail responses rather than crashing the server.

## License

MIT