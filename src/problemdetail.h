#ifndef PROBLEMDETAIL_H
#define PROBLEMDETAIL_H

#include <QString>
#include <QUrl>
#include <QJsonValue>
#include <QHttpServerResponse>

/**
 * @brief The ProblemDetail class implements the RFC 7807 Problem Details for HTTP APIs
 * 
 * This class provides a standardized way to report errors in HTTP APIs
 * according to the RFC 7807 specification.
 * 
 * @see https://tools.ietf.org/html/rfc7807
 */
class ProblemDetail
{
public:
    /**
     * @brief Constructs a ProblemDetail object with the given status code
     * 
     * @param statusCode The HTTP status code for this problem
     */
    explicit ProblemDetail(int statusCode = 500);

    /**
     * @brief Sets the type URI that identifies the problem type
     * 
     * @param type The URI that identifies the problem type
     */
    void setType(const QUrl &type);

    /**
     * @brief Sets the short, human-readable summary of the problem
     * 
     * @param title The summary of the problem
     */
    void setTitle(const QString &title);

    /**
     * @brief Sets a human-readable explanation specific to this occurrence of the problem
     * 
     * @param detail The explanation of the problem
     */
    void setDetail(const QString &detail);

    /**
     * @brief Sets a URI reference that identifies the specific occurrence of the problem
     * 
     * @param instance The URI reference for this problem instance
     */
    void setInstance(const QString &instance);

    /**
     * @brief Adds a custom extension property to the problem detail
     * 
     * @param key The key for the extension property
     * @param value The value for the extension property
     */
    void addExtension(const QString &key, const QJsonValue &value);

    /**
     * @brief Converts the problem detail to a QHttpServerResponse with the appropriate status code and content
     * 
     * @return A QHttpServerResponse representing the problem detail
     */
    QHttpServerResponse toJsonResponse() const;

private:
    int m_statusCode;
    QUrl m_type;
    QString m_title;
    QString m_detail;
    QString m_instance;
    QMap<QString, QJsonValue> m_extensions;
};

#endif // PROBLEMDETAIL_H