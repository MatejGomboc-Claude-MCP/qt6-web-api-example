# Qt6 Web API Example

A simple Qt6 C++ web API that returns 'Hello World' and supports RFC 7807 Problem Details for HTTP error reporting.

## Features

- Simple HTTP API built with Qt6's HttpServer module
- Returns "Hello World" on the root endpoint
- Full implementation of RFC 7807 Problem Details for standardized error reporting
- Example error routes to demonstrate problem detail responses

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
# Run with default settings (port 8080)
./qt6-web-api-example

# Run on a specific port
./qt6-web-api-example --port 3000
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

## License

MIT