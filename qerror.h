#ifndef QERROR_H
#define QERROR_H
#include <QString>
#include <QObject>

typedef enum _QError {
    NoError,
    Error,
    InvalidUrl,
    NetworkError,
    InvalidPath,
    FileOpenFailed,
    FileCreateFailed,
    ArchiveOpenFailed,
    UnZipFailed,
    AddFavoriteFailed
} QError;

typedef enum _QErrorMode {
    NO_MODE,
    APPEND,
} QErrorMode;

typedef struct _strArrEntry{ const QError err; const QString str; } strArrEntry;
static strArrEntry StrArr[] = {
    { Error, QT_TR_NOOP("Generic Error ") },
    { InvalidUrl, QT_TR_NOOP("Invalid URL ") },
    { NetworkError, QT_TR_NOOP("Network Error ") },
    { InvalidPath, QT_TR_NOOP("Invalid Path ") },
    { FileOpenFailed, QT_TR_NOOP("Failed to open file ") },
    { FileCreateFailed, QT_TR_NOOP("Failed to create file ") },
    { ArchiveOpenFailed, QT_TR_NOOP("Failed to open archive ") },
    { UnZipFailed, QT_TR_NOOP("Failed to open unzip archive ") },
    { AddFavoriteFailed, QT_TR_NOOP("Failed to add payload to favorites ") }
};

static const QString ErrorStr(QError err) {
    for (auto entry : StrArr) if (entry.err == err)
        return entry.str;
    return "";
}

typedef struct _QResult {
  QError err;
  QString str;

  _QResult(const QError error = NoError, const QString &errStr = "", const QErrorMode mode = NO_MODE) : err(error), str(errStr) {
      if (mode == APPEND)
          str = ErrorStr(err) + str;
  }

  const QString errorStr() {
    if (str.length())
        return str;
    return ErrorStr(err);
  }

  bool success() const { return err == NoError; }
} QResult;




/*
class QResult
{
public:
    QResult(QError err = NoError, const QString &errStr = "") : m_error(err), m_errorStr(errStr) {}
    QError error() { return m_error; }
    QString errorStr() {
        if (m_errorStr.length())
            return m_errorStr;
        return ErrorStr(m_error);
    }
    bool success() { return m_error == NoError; }

private:
    QError m_error;
    QString m_errorStr;


};
*/
#endif // QERROR_H
