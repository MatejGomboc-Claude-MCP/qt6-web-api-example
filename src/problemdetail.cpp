#include "problemdetail.h"
#include <QJsonObject>
#include <QJsonDocument>

// Initialize static member with default value
QString ProblemDetail::s_defaultBaseUrl = "https://problemdetails.example.com/problems";

ProblemDetail::ProblemDetail(int statusCode)
    : m_statusCode(statusCode)
{
    // Set default type URI based on status code and configurable base URL
    QString typeUri = QString("%1/%2").arg(s_defaultBaseUrl).arg(statusCode);
    m_type = QUrl(typeUri);
    
    // Set default title based on status code
    switch (statusCode) {
    case 400:
        m_title = "Bad Request";
        break;
    case 401:
        m_title = "Unauthorized";
        break;
    case 403:
        m_title = "Forbidden";
        break;
    case 404:
        m_title = "Not Found";
        break;
    case 405:
        m_title = "Method Not Allowed";
        break;
    case 409:
        m_title = "Conflict";
        break;
    case 422:
        m_title = "Unprocessable Entity";
        break;
    case 429:
        m_title = "Too Many Requests";
        break;
    case 500:
        m_title = "Internal Server Error";
        break;
    case 503:
        m_title = "Service Unavailable";
        break;
    default:
        m_title = "Unknown Error";
        break;
    }
}

void ProblemDetail::setType(const QUrl &type)
{
    m_type = type;
}

void ProblemDetail::setTitle(const QString &title)
{
    m_title = title;
}

void ProblemDetail::setDetail(const QString &detail)
{
    m_detail = detail;
}

void ProblemDetail::setInstance(const QString &instance)
{
    m_instance = instance;
}

void ProblemDetail::addExtension(const QString &key, const QJsonValue &value)
{
    m_extensions.insert(key, value);
}

QHttpServerResponse ProblemDetail::toJsonResponse() const
{
    QJsonObject jsonObject;
    
    // Add standard problem detail properties
    jsonObject["type"] = m_type.toString();
    jsonObject["title"] = m_title;
    jsonObject["status"] = m_statusCode;
    
    if (!m_detail.isEmpty()) {
        jsonObject["detail"] = m_detail;
    }
    
    if (!m_instance.isEmpty()) {
        jsonObject["instance"] = m_instance;
    }
    
    // Add any extension properties
    for (auto it = m_extensions.constBegin(); it != m_extensions.constEnd(); ++it) {
        jsonObject[it.key()] = it.value();
    }
    
    QJsonDocument document(jsonObject);
    auto response = QHttpServerResponse(document.toJson(), QHttpServerResponse::StatusCode(m_statusCode));
    response.setHeader("Content-Type", "application/problem+json");
    
    return response;
}

void ProblemDetail::setDefaultBaseUrl(const QString &baseUrl)
{
    s_defaultBaseUrl = baseUrl;
}

QString ProblemDetail::defaultBaseUrl()
{
    return s_defaultBaseUrl;
}