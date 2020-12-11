#ifndef QUTILS_H
#define QUTILS_H

#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QLocale>
#include <QtWidgets>
#include "qerror.h"

//class TegraRcmGUI;

template <typename T>
bool is_in(const T& v, std::initializer_list<T> lst)
{
    return std::find(std::begin(lst), std::end(lst), v) != std::end(lst);
}

typedef struct _Cpy {
    QString source;
    QString destination;
    _Cpy(const QString src, const QString dst) : source(src), destination(dst) {}
} Cpy;
typedef QList<Cpy> Cpys;

struct AppVersion {
    int major = 0;
    int minor = 0;
    int micro = 0;

    bool operator==(const AppVersion &other) const
    {
        return other.major == major && other.minor == minor && other.micro == micro;
    }

    bool operator<(const AppVersion &other) const
    {
        if (other.major != major)
            return other.major > major;
        else if (other.minor != minor)
            return other.minor > minor;
        else
            return other.micro > micro;
    }
};

QString GetAppVersionAsQString(AppVersion version);

enum fdMode { open_file, save_as };

QString FileDialog(QWidget *parent, fdMode mode, const QString& defaultName = "");
QString GetReadableSize(qint64 bytes);
QString GetStyleSheetFromResFile(QString qss_file);

class Badge : public QWidget {
    Q_OBJECT

public:
    Badge(QString label, QString value, QWidget* parent = nullptr);
    ~Badge();

private:
    QHBoxLayout *m_hbl;
    QLabel *m_l_label;
    QLabel *m_v_label;
};

class WarningBox : public QWidget {
        Q_OBJECT

public:
    WarningBox(QString message, QLayout *layout);
    ~WarningBox();

private:
    QLabel *m_label;
};

class Switch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)

public:
    Switch(bool initial_state, int width, QBrush brush = QBrush("#009688"), QWidget* parent = nullptr);
    Switch(const QBrush& brush, QWidget* parent = nullptr);

    QSize sizeHint() const override;

    QBrush brush() const {
        return _brush;
    }
    void setBrush(const QBrush &brsh) {
        _brush = brsh;
    }

    int offset() const {
        return _x;
    }
    void setOffset(int o) {
        _x = o;
        update();
    }

    bool isActive() { return _switch; }
    bool getState() { return _switch; }
    void setState(bool value) {  _switch = value; }
    void toggle();

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void enterEvent(QEvent*) override;

private:
    bool _switch;
    qreal _opacity;
    int _x, _y, _height, _margin, _width;
    QBrush _thumb, _track, _brush;
    QPropertyAnimation *_anim = nullptr;
};

class AnimatedLabel : public QLabel
{

  Q_OBJECT
  Q_PROPERTY(QColor color READ color WRITE setColor)

public:
  AnimatedLabel(QWidget *parent = 0)
  {
  }
  void setColor (QColor color){
    setStyleSheet(QString(
          "qproperty-alignment: 'AlignVCenter | AlignRight';"
          "background-color: rgba(%1, %2, %3, 200);"
          "color: rgb(0, 0, 0);"
          "border-top-left-radius: 20px; "
          "border-bottom-left-radius: 20px; "
          "padding: 10px;").arg(color.red()).arg(color.green()).arg(color.blue()));
  }
  QColor color(){
    return Qt::black; // getter is not really needed for now
  }
};

class MoveWindowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoveWindowWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    QPoint startPos;
};

void ClearLayout(QLayout *layout);

#endif // QUTILS_H
