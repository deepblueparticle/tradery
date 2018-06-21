/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <wininet.h>
#include "namevalue.h"

#ifdef MISCWIN_EXPORTS
#define MISCWIN_API __declspec(dllexport)
#else
#define MISCWIN_API __declspec(dllimport)
#endif

class CHandleException {
  /*
private:
  DWORD _lastError;

public:
  CHInternetException()
          : _lastError( ::GetLastError() )
  {
  }

  DWORD lastError() const { return _lastError; }
  */
};

class CHInternet {
 private:
  HINTERNET m_h;

 public:
  explicit CHInternet(HINTERNET h) : m_h(h) {}
  CHInternet() : m_h(0) {}

  ~CHInternet() {
    if (m_h != 0) {
      if (!InternetCloseHandle(m_h)) throw CHandleException();
    }
  }

  operator HINTERNET() const { return m_h; }
  operator bool() const { return m_h != 0; }
  void reset() { m_h = 0; }

  const CHInternet& operator=(CHInternet& h) {
    if (this != &h) {
      m_h = (HINTERNET)h;
      h.reset();
    }
    return *this;
  }
};

class InternetException {
 public:
  /**
   * Various error types
   */
  enum ErrorType {
    INTERNET_OPEN_ERROR,
    FAILED_TO_CONNECT,
    OPERATION_TIMEDOUT,
    HTTP_OPEN_REQUEST_ERROR,
    INTERNET_READ_FILE_ERROR,
    FAILED_TO_CLOSE_INTERNET_HANDLE,
    HTTP_SEND_REQUEST_EX_ERROR,
    INTERNET_WRITE_FILE_ERROR,
    HTTP_END_REQUEST_ERROR,
    HTTP_ADD_REQUEST_ERROR,
    HTTP_QUERY_INFO_ERROR,
    HTTP_RESPONSE_ERROR,
    REQUEST_CANCELLED,
    CURL_ERROR
  };

  enum ErrorCategory { CURL, HTTP, OTHER };

 private:
  const DWORD _lastError;
  const ErrorType _errorType;
  const unsigned int _httpResponseCode;
  const std::wstring _additionalInfo;

 private:
  ErrorCategory errorCategory(ErrorType errorType) const {
    static int errors[][2] = {{INTERNET_OPEN_ERROR, OTHER},
                              {FAILED_TO_CONNECT, OTHER},
                              {OPERATION_TIMEDOUT, OTHER},
                              {HTTP_OPEN_REQUEST_ERROR, HTTP},
                              {INTERNET_READ_FILE_ERROR, OTHER},
                              {FAILED_TO_CLOSE_INTERNET_HANDLE, OTHER},
                              {HTTP_SEND_REQUEST_EX_ERROR, HTTP},
                              {INTERNET_WRITE_FILE_ERROR, OTHER},
                              {HTTP_END_REQUEST_ERROR, HTTP},
                              {HTTP_ADD_REQUEST_ERROR, HTTP},
                              {HTTP_QUERY_INFO_ERROR, HTTP},
                              {HTTP_RESPONSE_ERROR, HTTP},
                              {REQUEST_CANCELLED, OTHER},
                              {CURL_ERROR, CURL}};

    for (unsigned int n = 0; n < sizeof(errors) / 2 / sizeof(int); ++n) {
      if (errorType == errors[n][0]) return (ErrorCategory)errors[n][1];
    }

    return OTHER;
  }

 public:
  bool curlError() const { return errorCategory(errorType()) == CURL; }
  bool httpError() const { return errorCategory(errorType()) == HTTP; }

  std::wstring message() const {
    switch (_errorType) {
      case INTERNET_OPEN_ERROR:
        return _T( "Internet open error" );
        break;
      case FAILED_TO_CONNECT:
        return _T( "failed to connect" );
        break;
      case OPERATION_TIMEDOUT:
        return _T( "operation timed out" );
        break;
      case HTTP_OPEN_REQUEST_ERROR:
        return _T( "HTTP open request error" );
        break;
      case INTERNET_READ_FILE_ERROR:
        return _T( "Internet read file error" );
        break;
      case FAILED_TO_CLOSE_INTERNET_HANDLE:
        return _T( "Failded to close internet handle" );
        break;
      case HTTP_SEND_REQUEST_EX_ERROR:
        return _T( "HTTP send request ex error (no Internet connection available)" );
        break;
      case INTERNET_WRITE_FILE_ERROR:
        return _T( "Internet write file error" );
        break;
      case HTTP_END_REQUEST_ERROR:
        return _T( "HTTP end request error" );
        break;
      case HTTP_ADD_REQUEST_ERROR:
        return _T( "HTTP add request error" );
        break;
      case HTTP_QUERY_INFO_ERROR:
        return _T( "HTTP query info error" );
        break;
      case HTTP_RESPONSE_ERROR:
        return _T( "HTTP response error" );
        break;
      case REQUEST_CANCELLED:
        return _T( "Request canceled" );
        break;
      case CURL_ERROR: {
        std::wostringstream os;

        os << _T( "Curl error: " ) << _additionalInfo << _T( " " )
           << httpResponseCode();
        return os.str();
      }
      default:
        return _T( "Unknown internet error" );
        break;
    }
  }
  /**
   * Constructor for InternetException - takes the error type as parameter
   *
   * @param errorType Error type
   */
  InternetException(ErrorType errorType, unsigned int httpResponseCode = 0,
                    const std::wstring& additionalInfo = std::wstring())
      : _lastError(GetLastError()),
        _errorType(errorType),
        _httpResponseCode(httpResponseCode),
        _additionalInfo(additionalInfo) {}

  /**
   * Returns the last error, i.e. the value set by the call to GetLastError
   *
   * @return Value returned by GetLastError
   */
  DWORD lastError() const { return _lastError; }
  /**
   * Returns the error type
   *
   * @return The error type
   */
  ErrorType errorType() const { return _errorType; }
  unsigned int httpResponseCode() const { return _httpResponseCode; }
};

class HTTPRequestBase {
 private:
  const std::wstring m_serverName;
  bool m_http;  // true for http, false for https

 public:
  HTTPRequestBase(const std::wstring& serverName, bool http = true)
      : m_serverName(serverName), m_http(http) {}

  virtual ~HTTPRequestBase() {}

  virtual tradery::StringPtr get(
      const std::wstring& object,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0) = 0;

  tradery::StringPtr get(
      const std::wstring& object, const tradery::NameValueMap& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0) {
    std::wstring query = object;

    if (!query.empty() && query.find(_T( "?" )) == std::wstring::npos)
      query += _T("?");

    query += data.toString();

    return get(query, extraHeaders, userAgent, proxyServerAddress,
               proxyServerUserName, proxyServerPassword, timeout);
  }

  virtual tradery::StringPtr post(
      const std::wstring& object, const tradery::NameValueMap& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0) = 0;

  bool operator==(const std::wstring& serverName) const {
    return serverName == m_serverName;
  }
  bool operator!=(const std::wstring& serverName) const {
    return serverName != m_serverName;
  }
  virtual void cancel() = 0;
  virtual void unCancel() = 0;

 protected:
  const std::wstring& getServerName() const { return m_serverName; }
  std::wstring protocol() const {
    return m_http ? _T( "http" ) : _T( "https" );
  }
  std::wstring url(const std::wstring& object);
};

typedef tradery::ManagedPtr<HTTPRequestBase> HTTPRequestBasePtr;

/**
 * Encapsulation of a HTTP request
 *
 * Currently does post, can be extended
 *
 * @author Adrian Michel
 */
class MISCWIN_API CHTTPRequest : public HTTPRequestBase {
 public:
  /**
   * Exception thrown by CHTTPRequest members.
   *
   * Contains an error code and the last error as returned by GetLastError
   */

 private:
  CHInternet m_hSession;
  CHInternet m_hConnect;
  const std::wstring& m_doc;

  tradery::ManagedPtr<std::wstring> _response;
  enum { BUFFSIZE = 500 };
  tradery::Mutex _mx;
  bool _cancelling;

 public:
  /**
   * Constructor - takes a string to be sent to the server,
   * the server name and a server "object".
   *
   * The doc has to be UTF8 encoded.
   *
   * For example, the server url is: http://www.plaxo.com/xml2xml/axis/calendar
   * ,
   * - server name: www.plaxo.com
   * - object: xml2xml/axis/calendar
   *
   * @param doc        UTF8 encoded string to be sent to the server
   * @param serverName server name (e.g. "www.plaxo.com")
   * @param objectName The part in the URL after the server name
   */
  CHTTPRequest(const std::wstring& serverName,
               const std::wstring& doc = std::wstring());
  /**
   * Do a post and return the server response as a UTF8 encoded string
   *
   * @return the server response, UTF8 encoded
   * @exception InternetException
   */
  virtual tradery::StringPtr get(
      const std::wstring& object,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0);

  virtual tradery::StringPtr post(
      const std::wstring& object, const tradery::NameValueMap& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0) {
    // not implemented
    assert(false);
    return tradery::StringPtr();
  }

  operator const std::wstring&() const { return *_response; }
  bool cancelling() {
    tradery::Lock lock(_mx);
    return _cancelling;
  }

  void resetCancel() {
    tradery::Lock lock(_mx);
    _cancelling = false;
  }

  void cancel() {
    tradery::Lock lock(_mx);
    _cancelling = true;
  }

  void unCancel() {
    tradery::Lock lock(_mx);
    _cancelling = false;
  }

 private:
  void UseHttpSendReqEx(HINTERNET hRequest) const throw(InternetException);

  void setVerify(bool verifyPeer, bool verifyHost);
};

typedef tradery::ManagedPtr<CHTTPRequest> CHTTPRequestPtr;

class MISCWIN_API CurlHTTPRequest : public HTTPRequestBase {
 private:
  void* _curl;
  tradery::Mutex _mx;
  bool _cancel;
  bool _canceled;
  bool m_verifyPeer;
  bool m_verifyHost;

  static int xprogress(void* req, double t, double d, double ultotal,
                       double ulnow);
  void handleCurlResult(unsigned int res, const char* name);

  int progress();

  void setVerify();

 public:
  CurlHTTPRequest(const std::wstring& serverName, bool http = true,
                  bool verifyPeer = false, bool verifyHost = false);
  virtual ~CurlHTTPRequest();

  virtual tradery::StringPtr get(
      const std::wstring& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0);

  virtual tradery::StringPtr post(
      const std::wstring& object, const tradery::NameValueMap& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0);

  tradery::StringPtr get(
      const std::wstring& object, const tradery::NameValueMap& data,
      const std::wstring& extraHeaders = std::wstring(),
      const std::wstring& userAgent = std::wstring(),
      const std::wstring& proxyServerAddress = std::wstring(),
      const std::wstring& proxyServerUserName = std::wstring(),
      const std::wstring& proxyServerPassword = std::wstring(),
      unsigned int timeout = 0) {
    return __super::get(object, data, extraHeaders, userAgent,
                        proxyServerAddress, proxyServerUserName,
                        proxyServerPassword, timeout);
  }

  virtual void cancel();
  void unCancel();
};
