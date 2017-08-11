#ifndef QCURL_H
#define QCURL_H

#include "qcurl_global.h"
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qscopedpointer.h>
#include <QObject>

QT_BEGIN_HEADER

class QTcpSocket;
class QTimerEvent;
class QIODevice;
class QCurlAuthenticator;
class QNetworkProxy;
class QSslError;

class QCurlPrivate;

class QCurlHeaderPrivate;
class QCurlHeader
{
public:
    QCurlHeader();
    QCurlHeader(const QCurlHeader &header);
    QCurlHeader(const QString &str);
    virtual ~QCurlHeader();

    QCurlHeader &operator=(const QCurlHeader &h);

    void setValue(const QString &key, const QString &value);
    void setValues(const QList<QPair<QString, QString> > &values);
    void addValue(const QString &key, const QString &value);
    QList<QPair<QString, QString> > values() const;
    bool hasKey(const QString &key) const;
    QStringList keys() const;
    QString value(const QString &key) const;
    QStringList allValues(const QString &key) const;
    void removeValue(const QString &key);
    void removeAllValues(const QString &key);

    // ### Qt 5: change to qint64
    bool hasContentLength() const;
    uint contentLength() const;
    void setContentLength(int len);

    bool hasContentType() const;
    QString contentType() const;
    void setContentType(const QString &type);

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine(const QString &line, int number);
    bool parse(const QString &str);
    void setValid(bool);

    QCurlHeader(QCurlHeaderPrivate &dd, const QString &str = QString());
    QCurlHeader(QCurlHeaderPrivate &dd, const QCurlHeader &header);
    QScopedPointer<QCurlHeaderPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(QCurlHeader)
};

class QCurlResponseHeaderPrivate;
class QCurlResponseHeader : public QCurlHeader
{
public:
    QCurlResponseHeader();
    QCurlResponseHeader(const QCurlResponseHeader &header);
    QCurlResponseHeader(const QString &str);
    QCurlResponseHeader(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);
    QCurlResponseHeader &operator=(const QCurlResponseHeader &header);

    void setStatusLine(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QCurlResponseHeader)
    friend class QCurlPrivate;
};

class QCurlRequestHeaderPrivate;
class QCurlRequestHeader : public QCurlHeader
{
public:
    QCurlRequestHeader();
    QCurlRequestHeader(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);
    QCurlRequestHeader(const QCurlRequestHeader &header);
    QCurlRequestHeader(const QString &str);
    QCurlRequestHeader &operator=(const QCurlRequestHeader &header);

    void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

    QString method() const;
    QString path() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QCurlRequestHeader)
};

class QCURLSHARED_EXPORT QCurl : public QObject
{
    Q_OBJECT

public:
    enum ConnectionMode {
        ConnectionModeHttp,
        ConnectionModeHttps
    };

    explicit QCurl(QObject *parent = 0);
    QCurl(const QString &hostname, quint16 port = 80, QObject *parent = 0);
    QCurl(const QString &hostname, ConnectionMode mode, quint16 port = 0, QObject *parent = 0);
    virtual ~QCurl();

    enum State {
        Unconnected,
        HostLookup,
        Connecting,
        Sending,
        Reading,
        Connected,
        Closing
    };
    enum Error {
        NoError,
        UnknownError,
        HostNotFound,
        ConnectionRefused,
        UnexpectedClose,
        InvalidResponseHeader,
        WrongContentLength,
        Aborted,
        AuthenticationRequiredError,
        ProxyAuthenticationRequiredError
    };

    int setHost(const QString &hostname, quint16 port = 80);
    int setHost(const QString &hostname, ConnectionMode mode, quint16 port = 0);

    int setSocket(QTcpSocket *socket);
    int setUser(const QString &username, const QString &password = QString());

#ifndef QT_NO_NETWORKPROXY
    int setProxy(const QString &host, int port,
                 const QString &username = QString(),
                 const QString &password = QString());
    int setProxy(const QNetworkProxy &proxy);
#endif

    int get(const QString &path, QIODevice *to=0);
    int post(const QString &path, QIODevice *data, QIODevice *to=0 );
    int post(const QString &path, const QByteArray &data, QIODevice *to=0);
    int head(const QString &path);
    int request(const QCurlRequestHeader &header, QIODevice *device=0, QIODevice *to=0);
    int request(const QCurlRequestHeader &header, const QByteArray &data, QIODevice *to=0);

    int closeConnection();
    int close();

    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();

    int currentId() const;
    QIODevice *currentSourceDevice() const;
    QIODevice *currentDestinationDevice() const;
    QCurlRequestHeader currentRequest() const;
    QCurlResponseHeader lastResponse() const;
    bool hasPendingRequests() const;
    void clearPendingRequests();

    State state() const;

    Error error() const;
    QString errorString() const;

public Q_SLOTS:
    void abort();

#ifndef QT_NO_OPENSSL
    void ignoreSslErrors();
#endif

Q_SIGNALS:
    void stateChanged(int);
    void responseHeaderReceived(const QCurlResponseHeader &resp);
    void readyRead(const QCurlResponseHeader &resp);

    // ### Qt 5: change to qint64
    void dataSendProgress(int, int);
    void dataReadProgress(int, int);

    void requestStarted(int);
    void requestFinished(int, bool);
    void done(bool);

#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QCurlAuthenticator *);
#endif
    void authenticationRequired(const QString &hostname, quint16 port, QCurlAuthenticator *);

#ifndef QT_NO_OPENSSL
    void sslErrors(const QList<QSslError> &errors);
#endif

private:
    Q_DISABLE_COPY(QCurl)
    QScopedPointer<QCurlPrivate> d;

    Q_PRIVATE_SLOT(d, void _q_startNextRequest())
    Q_PRIVATE_SLOT(d, void _q_slotReadyRead())
    Q_PRIVATE_SLOT(d, void _q_slotConnected())
    Q_PRIVATE_SLOT(d, void _q_slotError(QAbstractSocket::SocketError))
    Q_PRIVATE_SLOT(d, void _q_slotClosed())
    Q_PRIVATE_SLOT(d, void _q_slotBytesWritten(qint64 numBytes))
    Q_PRIVATE_SLOT(d, void _q_slotEncryptedBytesWritten(qint64 numBytes))
    Q_PRIVATE_SLOT(d, void _q_slotDoFinished())
    Q_PRIVATE_SLOT(d, void _q_slotSendRequest())
    Q_PRIVATE_SLOT(d, void _q_continuePost())

    friend class QCurlNormalRequest;
    friend class QCurlSetHostRequest;
    friend class QCurlSetSocketRequest;
    friend class QCurlSetUserRequest;
    friend class QCurlSetProxyRequest;
    friend class QCurlCloseRequest;
    friend class QCurlPGHRequest;
};

QT_END_HEADER


#endif // QCURL_H
