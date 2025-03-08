#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QStringList>
#include <QHostAddress>

/**
 * @brief The ConfigManager class handles loading and providing access to configuration settings
 * 
 * This class is responsible for loading configuration from a JSON file and providing
 * an interface to access the configuration values.
 */
class ConfigManager
{
public:
    /**
     * @brief Constructs a ConfigManager with default configuration
     */
    ConfigManager();
    
    /**
     * @brief Loads configuration from the specified JSON file
     * 
     * @param configPath Path to the configuration file
     * @return true if the configuration was loaded successfully, false otherwise
     */
    bool loadConfig(const QString &configPath);
    
    /**
     * @brief Overrides configuration values with command line arguments
     * 
     * @return true if the arguments were processed successfully, false otherwise
     */
    bool processCommandLine();
    
    // Server settings
    int getPort() const;
    QHostAddress getAddress() const;
    int getWorkers() const;
    
    // Rate limiting settings
    bool isRateLimitEnabled() const;
    int getMaxRequestsPerMinute() const;
    QStringList getRateLimitIpWhitelist() const;
    
    // CORS settings
    bool isCorsEnabled() const;
    QStringList getAllowedOrigins() const;
    QStringList getAllowedMethods() const;
    QStringList getAllowedHeaders() const;
    int getCorsMaxAge() const;
    
    // TLS settings
    bool isTlsEnabled() const;
    QString getCertificatePath() const;
    QString getKeyPath() const;
    QString getPassphrase() const;
    
    // Security headers
    QString getContentTypeOptions() const;
    QString getFrameOptions() const;
    QString getContentSecurityPolicy() const;
    int getHstsMaxAge() const;
    bool getHstsIncludeSubdomains() const;
    
    // Problem details
    QString getProblemBaseUrl() const;
    bool includeDebugInfo() const;
    QString getContactEmail() const;
    
    // Logging
    QString getLogLevel() const;
    QString getLogFile() const;
    bool isConsoleLoggingEnabled() const;
    bool includeTimestamp() const;
    
private:
    QJsonObject m_config;
    QString m_configPath;
    
    // Helper methods to get values from the config with defaults
    QString getString(const QStringList &path, const QString &defaultValue) const;
    int getInt(const QStringList &path, int defaultValue) const;
    bool getBool(const QStringList &path, bool defaultValue) const;
    QStringList getStringList(const QStringList &path, const QStringList &defaultValue) const;
    
    // Set up default configuration
    void setDefaults();
};

#endif // CONFIGMANAGER_H