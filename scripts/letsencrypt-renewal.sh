#!/bin/bash
#
# Let's Encrypt certificate renewal script for Qt6 Web API Example
# For Debian Linux
#
# This script should be run as a cron job to automatically renew Let's Encrypt SSL certificates
# and update the application's configuration file.
#
# Recommended cron schedule: twice daily (recommended by Let's Encrypt)
# 0 0,12 * * * /path/to/qt6-web-api-example/scripts/letsencrypt-renewal.sh >> /var/log/letsencrypt-renewal.log 2>&1
#

set -e

# Configuration
DOMAIN="example.com"                                   # Your domain
EMAIL="admin@example.com"                              # Your email for Let's Encrypt notifications
APP_DIR="/opt/qt6-web-api-example"                     # Application directory
CONFIG_FILE="$APP_DIR/config.json"                     # Path to application config.json
SERVICE_NAME="qt6-web-api"                             # Systemd service name (if using systemd)
CERT_DIR="/etc/letsencrypt/live/$DOMAIN"               # Let's Encrypt cert directory
LOG_FILE="/var/log/letsencrypt-renewal.log"            # Log file location

# Output timestamp for logging
echo "--- Let's Encrypt renewal started at $(date) ---"

# Check if certbot is installed
if ! command -v certbot &> /dev/null; then
    echo "Certbot not found. Installing..."
    apt-get update
    apt-get install -y certbot
fi

# Run certbot renewal
echo "Running Let's Encrypt certificate renewal..."
certbot renew --non-interactive --quiet

# Check if renewal was successful by looking at certificate dates
if [ -f "$CERT_DIR/fullchain.pem" ]; then
    CERT_EXPIRY=$(openssl x509 -enddate -noout -in "$CERT_DIR/fullchain.pem" | cut -d= -f2)
    echo "Certificate valid until: $CERT_EXPIRY"
    
    # Update application configuration file with certificate paths if needed
    if [ -f "$CONFIG_FILE" ]; then
        echo "Updating application configuration..."
        # Check if the config already has the correct paths
        CERT_PATH_CONFIGURED=$(grep -c "\"certificatePath\": \"$CERT_DIR/fullchain.pem\"" "$CONFIG_FILE" || true)
        KEY_PATH_CONFIGURED=$(grep -c "\"keyPath\": \"$CERT_DIR/privkey.pem\"" "$CONFIG_FILE" || true)
        TLS_ENABLED=$(grep -c "\"enabled\": true" "$CONFIG_FILE" || true)
        
        # Create a temporary file for configuration updates
        TMP_CONFIG=$(mktemp)
        
        if [ "$CERT_PATH_CONFIGURED" -eq 0 ] || [ "$KEY_PATH_CONFIGURED" -eq 0 ] || [ "$TLS_ENABLED" -eq 0 ]; then
            # Use jq to update the configuration if available
            if command -v jq &> /dev/null; then
                echo "Using jq to update configuration..."
                jq '.security.tls.enabled = true | 
                    .security.tls.certificatePath = "'$CERT_DIR'/fullchain.pem" | 
                    .security.tls.keyPath = "'$CERT_DIR'/privkey.pem"' "$CONFIG_FILE" > "$TMP_CONFIG"
                mv "$TMP_CONFIG" "$CONFIG_FILE"
            else
                echo "jq not found. Installing..."
                apt-get update
                apt-get install -y jq
                
                echo "Using jq to update configuration..."
                jq '.security.tls.enabled = true | 
                    .security.tls.certificatePath = "'$CERT_DIR'/fullchain.pem" | 
                    .security.tls.keyPath = "'$CERT_DIR'/privkey.pem"' "$CONFIG_FILE" > "$TMP_CONFIG"
                mv "$TMP_CONFIG" "$CONFIG_FILE"
            fi
            
            echo "Configuration updated."
        else
            echo "Configuration already contains correct certificate paths."
            rm "$TMP_CONFIG"
        fi
        
        # Set proper permissions for the config file
        chown $(stat -c "%U:%G" "$APP_DIR") "$CONFIG_FILE"
        chmod 644 "$CONFIG_FILE"
        
        # Restart the application service if using systemd
        if systemctl is-active --quiet "$SERVICE_NAME"; then
            echo "Restarting application service..."
            systemctl restart "$SERVICE_NAME"
        else
            echo "Service $SERVICE_NAME not found or not active."
            echo "If you're using systemd, configure it with:"
            echo "systemctl enable $SERVICE_NAME"
            echo "systemctl start $SERVICE_NAME"
        fi
    else
        echo "Warning: Configuration file $CONFIG_FILE not found"
    fi
else
    echo "Error: Certificate files not found at $CERT_DIR"
    exit 1
fi

echo "--- Let's Encrypt renewal completed at $(date) ---"
