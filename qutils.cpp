#include "qutils.h"


QString FileDialog(QWidget *parent, fdMode mode, const QString& defaultName)
{
    QFileDialog fd(parent);
    QString filePath;
    if (mode == open_file)
    {
        filePath = QFileDialog::getOpenFileName(parent, "Open file", "default_dir\\");
    }
    else
    {
        fd.setAcceptMode(QFileDialog::AcceptSave); // Ask overwrite
        filePath = fd.getSaveFileName(parent, "Save as", "default_dir\\" + defaultName);
    }
    if (!filePath.isEmpty())
    {
        QSettings appSettings;
        QDir CurrentDir;
        appSettings.setValue("default_dir", CurrentDir.absoluteFilePath(filePath));
    }
    return filePath;
}

QString GetReadableSize(qint64 bytes)
{
    return QLocale().formattedDataSize(bytes);
}

Switch::Switch(bool initial_state, int width, QBrush brush, QWidget *parent) : QAbstractButton(parent),
_height(16),
_opacity(0.000),
_switch(initial_state),
_margin(3),
_thumb("#d5d5d5"),
_width(width),
_anim(new QPropertyAnimation(this, "offset", this))
{
    if (!initial_state)
    {
        setOffset(_height / 2);
    }
    else
    {
        setOffset(_width - _height);
        _thumb = brush;
    }
    _y = _height / 2;
    setBrush(brush);
}

Switch::Switch(const QBrush &brush, QWidget *parent) : QAbstractButton(parent),
_height(16),
_switch(false),
_opacity(0.000),
_margin(3),
_thumb("#d5d5d5"),
_anim(new QPropertyAnimation(this, "offset", this))
{
    setOffset(_height / 2);
    _y = _height / 2;
    setBrush(brush);
}

void Switch::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.setPen(Qt::NoPen);
    if (isEnabled()) {
        p.setBrush(_switch ? brush() : Qt::black);
        p.setOpacity(_switch ? 0.5 : 0.38);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawRoundedRect(QRect(_margin, _margin, _width - 2 * _margin, height() - 2 * _margin), 8.0, 8.0);
        p.setBrush(_thumb);
        p.setOpacity(1.0);
        p.drawEllipse(QRectF(offset() - (_height / 2), _y - (_height / 2), height(), height()));

    } else {
        p.setBrush(Qt::black);
        p.setOpacity(0.12);
        p.drawRoundedRect(QRect(_margin, _margin, _width - 2 * _margin, height() - 2 * _margin), 8.0, 8.0);
        p.setOpacity(1.0);
        p.setBrush(QColor("#BDBDBD"));
        p.drawEllipse(QRectF(offset() - (_height / 2), _y - (_height / 2), height(), height()));
    }
}

void Switch::toggle()
{
    int toffset = offset();
    _switch = _switch ? false : true;
    _thumb = _switch ? _brush : QBrush("#d5d5d5");
    if (_switch) {
        _anim->setStartValue(_height / 2);
        _anim->setEndValue(_width - _height);
        _anim->setDuration(120);
        _anim->start();
    } else {
        _anim->setStartValue(offset());
        _anim->setEndValue(_height / 2);
        _anim->setDuration(120);
        _anim->start();
    }
}

void Switch::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() & Qt::LeftButton) {
        toggle();
    }
    QAbstractButton::mouseReleaseEvent(e);
}

void Switch::enterEvent(QEvent *e) {
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::enterEvent(e);
}

QSize Switch::sizeHint() const {
    return QSize(2 * (_height + _margin), _height + 2 * _margin);
}

QString GetStyleSheetFromResFile(QString qss_file)
{
    QFile File(qss_file);
    File.open(QFile::ReadOnly);
    QString StyleSheet = QLatin1String(File.readAll());
    File.close();
    return StyleSheet;
}


MoveWindowWidget::MoveWindowWidget(QWidget *parent) : QWidget(parent)
{
}

void MoveWindowWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void MoveWindowWidget::mousePressEvent(QMouseEvent *event)
{
    startPos = event->pos();
    QWidget::mousePressEvent(event);
}

void MoveWindowWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - startPos;
    QWidget * w = window();
    if(w)
        w->move(w->pos() + delta);
    QWidget::mouseMoveEvent(event);
}
