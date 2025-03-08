# Maintenance Scripts

This directory contains utility scripts for maintaining and deploying the Qt6 Web API Example application.

## Let's Encrypt SSL Certificate Renewal

### Overview

The `letsencrypt-renewal.sh` script automates the process of renewing Let's Encrypt SSL certificates for your application. Let's Encrypt certificates are valid for 90 days, and it's recommended to renew them every 60 days.

### Prerequisites

- Debian Linux (latest version)
- Root or sudo access
- `certbot` will be automatically installed by the script if needed
- `jq` will be automatically installed by the script if needed

### Initial Setup

1. **Install certbot and obtain your first certificate**:

   ```bash
   sudo apt-get update
   sudo apt-get install -y certbot
   
   # For standard HTTP validation (requires port 80 to be open)
   sudo certbot certonly --standalone -d example.com -d www.example.com --agree-tos --email admin@example.com
   
   # Or for DNS validation (if you can't open port 80)
   sudo certbot certonly --manual --preferred-challenges dns -d example.com -d www.example.com --agree-tos --email admin@example.com
   ```

2. **Modify the configuration in the script**:

   Edit the script to customize the following variables:
   - `DOMAIN`: Your domain name
   - `EMAIL`: Your email address for Let's Encrypt notifications
   - `APP_DIR`: Directory where your application is installed
   - `CONFIG_FILE`: Path to your application's config.json
   - `SERVICE_NAME`: Name of the systemd service if you're using systemd

3. **Make the script executable**:

   ```bash
   sudo chmod +x /path/to/qt6-web-api-example/scripts/letsencrypt-renewal.sh
   ```

4. **Test the script**:

   ```bash
   sudo /path/to/qt6-web-api-example/scripts/letsencrypt-renewal.sh
   ```

### Setting up the Cron Job

Let's Encrypt recommends attempting renewal twice per day. Most renewals will be no-ops until the certificate is near expiry.

1. **Edit the crontab**:

   ```bash
   sudo crontab -e
   ```

2. **Add the following line**:

   ```
   0 0,12 * * * /path/to/qt6-web-api-example/scripts/letsencrypt-renewal.sh >> /var/log/letsencrypt-renewal.log 2>&1
   ```

   This will run the script at midnight and noon every day.

3. **Create the log file**:

   ```bash
   sudo touch /var/log/letsencrypt-renewal.log
   sudo chmod 644 /var/log/letsencrypt-renewal.log
   ```

### Setting Up As a Systemd Service

If you prefer to run the application as a systemd service (recommended for production deployments):

1. **Create a systemd service file**:

   ```bash
   sudo nano /etc/systemd/system/qt6-web-api.service
   ```

   Add the following content (customize as needed):

   ```
   [Unit]
   Description=Qt6 Web API Example
   After=network.target

   [Service]
   Type=simple
   User=www-data
   WorkingDirectory=/opt/qt6-web-api-example
   ExecStart=/opt/qt6-web-api-example/qt6-web-api-example
   Restart=on-failure
   RestartSec=5
   StandardOutput=journal
   StandardError=journal

   [Install]
   WantedBy=multi-user.target
   ```

2. **Enable and start the service**:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable qt6-web-api
   sudo systemctl start qt6-web-api
   ```

3. **Check the service status**:

   ```bash
   sudo systemctl status qt6-web-api
   ```

### Firewall Configuration

Make sure your firewall allows:
- Port 80 (HTTP) - Required for Let's Encrypt domain validation
- Port 443 (HTTPS) - Required for your HTTPS API

For UFW (Uncomplicated Firewall):

```bash
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
```

### Troubleshooting

1. **Check the renewal logs**:
   ```bash
   sudo tail -n 50 /var/log/letsencrypt-renewal.log
   ```

2. **Check certbot logs**:
   ```bash
   sudo tail -n 50 /var/log/letsencrypt/letsencrypt.log
   ```

3. **Test certificate renewal manually**:
   ```bash
   sudo certbot renew --dry-run
   ```

4. **Verify certificate paths**:
   ```bash
   sudo ls -la /etc/letsencrypt/live/your-domain.com/
   ```

5. **Check application logs**:
   ```bash
   sudo journalctl -u qt6-web-api
   ```
