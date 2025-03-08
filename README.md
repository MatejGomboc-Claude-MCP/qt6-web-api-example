# Qt6 Web API Example

A simple Qt6 C++ web API that returns 'Hello World' and supports RFC 7807 Problem Details for HTTP error reporting.

## Features

- Simple HTTP API built with Qt6's HttpServer module
- Returns "Hello World" on the root endpoint
- Full implementation of RFC 7807 Problem Details for standardized error reporting
- JSON configuration file for easy deployment and customization
- Comprehensive security features:
  - OWASP recommended security headers
  - Automatic HTTP to HTTPS redirection
  - Rate limiting to prevent abuse
  - TLS/HTTPS support with Let's Encrypt integration
  - CORS support for web clients
  - Exception safety with proper error handling
  - Configurable problem detail format
  - Local-only binding by default (for improved security)
- Deployment and maintenance utilities:
  - Let's Encrypt certificate renewal automation
  - Systemd service configuration

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
    "workers": 4,
    "httpRedirect": {
      "enabled": false,
      "httpPort": 80
    }
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
      "permissionsPolicy": "geolocation=(), camera=(), microphone=()",
      "referrerPolicy": "strict-origin-when-cross-origin",
      "xssProtection": "1; mode=block",
      "hstsMaxAge": 31536000,
      "hstsIncludeSubdomains": true,
      "cacheControl": "no-store, max-age=0",
      "clearSiteData": "",
      "crossOriginEmbedderPolicy": "require-corp",
      "crossOriginOpenerPolicy": "same-origin",
      "crossOriginResourcePolicy": "same-origin"
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

# Enable HTTP to HTTPS redirect
./qt6-web-api-example --tls true --cert /path/to/cert.pem --key /path/to/key.pem --http-redirect true --http-port 80
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

### HTTP to HTTPS Redirection

When TLS is enabled, the API can automatically redirect all HTTP traffic to HTTPS, ensuring secure communications. This feature can be configured in the JSON configuration file:

```json
"httpRedirect": {
  "enabled": true,
  "httpPort": 80
}
```

Or via command line:

```bash
./qt6-web-api-example --tls true --http-redirect true --http-port 80
```

The redirect uses a proper 301 Moved Permanently status code to ensure browsers and clients update their bookmarks and caches.

### OWASP Recommended Security Headers

The API implements recommended security headers from the [OWASP HTTP Headers Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/HTTP_Headers_Cheat_Sheet.html), including:

- **Content-Type Options**: Prevents MIME-type sniffing
  ```
  X-Content-Type-Options: nosniff
  ```

- **Frame Options**: Prevents clickjacking attacks
  ```
  X-Frame-Options: DENY
  ```

- **Content Security Policy**: Limits sources of content
  ```
  Content-Security-Policy: default-src 'self'
  ```

- **Permissions Policy**: Controls browser features
  ```
  Permissions-Policy: geolocation=(), camera=(), microphone=()
  ```

- **Referrer Policy**: Controls information in the Referer header
  ```
  Referrer-Policy: strict-origin-when-cross-origin
  ```

- **XSS Protection**: Additional XSS mitigation for older browsers
  ```
  X-XSS-Protection: 1; mode=block
  ```

- **HSTS**: Forces HTTPS connections (when TLS is enabled)
  ```
  Strict-Transport-Security: max-age=31536000; includeSubDomains
  ```

- **Cache Control**: Prevents caching of sensitive information
  ```
  Cache-Control: no-store, max-age=0
  ```

- **Cross-Origin Policies**: Restricts cross-origin interactions
  ```
  Cross-Origin-Embedder-Policy: require-corp
  Cross-Origin-Opener-Policy: same-origin
  Cross-Origin-Resource-Policy: same-origin
  ```

All security headers are configurable through the JSON configuration file.

### Rate Limiting

The API includes IP-based rate limiting to prevent abuse. By default, each client IP is limited to 100 requests per minute. You can configure this in the JSON configuration:

```json
"rateLimit": {
  "enabled": true,
  "maxRequestsPerMinute": 100,
  "ipWhitelist": ["127.0.0.1", "::1"]
}
```

### TLS/HTTPS Support with Let's Encrypt

For production use, enable TLS in the configuration:

```json
"tls": {
  "enabled": true,
  "certificatePath": "/etc/letsencrypt/live/example.com/fullchain.pem",
  "keyPath": "/etc/letsencrypt/live/example.com/privkey.pem",
  "passphrase": ""
}
```

#### Let's Encrypt Certificate Automation

This project includes scripts for automating Let's Encrypt certificate issuance and renewal on Debian-based Linux systems. The scripts are located in the `scripts/` directory:

- `letsencrypt-renewal.sh`: Automates the renewal of Let's Encrypt certificates and updates the application configuration
- `qt6-web-api.service`: Example systemd service file for running the API as a service

For detailed setup instructions, see the [Scripts README](scripts/README.md).

Quick setup:

```bash
# Initial certificate acquisition
sudo certbot certonly --standalone -d example.com -d www.example.com

# Set up automated renewal
sudo cp scripts/letsencrypt-renewal.sh /usr/local/bin/
sudo chmod +x /usr/local/bin/letsencrypt-renewal.sh
sudo crontab -e
# Add: 0 0,12 * * * /usr/local/bin/letsencrypt-renewal.sh >> /var/log/letsencrypt-renewal.log 2>&1
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

### Exception Handling

All routes include comprehensive exception handling to ensure that unexpected errors are properly caught and returned as ProblemDetail responses rather than crashing the server. This enhances both security and reliability by providing consistent error handling across the entire API.

## Production Deployment

For production deployments, we recommend:

1. Run as a systemd service (see `scripts/qt6-web-api.service`)
2. Enable TLS with Let's Encrypt certificates
3. Set up the automated certificate renewal script (see `scripts/letsencrypt-renewal.sh`)
4. Configure HTTP to HTTPS redirection
5. Use a non-root user for running the service

Setup example:

```bash
# Install in production location
sudo mkdir -p /opt/qt6-web-api-example
sudo cp -r build/qt6-web-api-example config.json /opt/qt6-web-api-example/

# Set up systemd service
sudo cp scripts/qt6-web-api.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable qt6-web-api
sudo systemctl start qt6-web-api
```

## Performance Considerations

The OWASP security headers have been carefully selected to provide strong security while minimizing performance impact:

- Headers are configured once at startup and applied consistently
- The ConfigManager uses efficient string lookups with sensible defaults
- Memory usage is minimized by reusing configuration objects
- Header application is performed in the response pipeline without blocking

For high-traffic deployments, consider:

1. Increasing the `workers` setting in the configuration
2. Adjusting rate limits for your specific use case
3. Using a reverse proxy like Nginx for TLS termination and additional caching

## Extending the API

This project provides a solid foundation that you can extend:

1. Add new routes in `apiserver.cpp`
2. Add authentication by implementing a middleware in the request pipeline
3. Add database integration by connecting to your preferred database
4. Implement logging by extending the configuration and adding a logging facility

## License

MIT