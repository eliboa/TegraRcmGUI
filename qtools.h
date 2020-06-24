#ifndef QTOOLS_H
#define QTOOLS_H

#include <QWidget>

namespace Ui {
class qTools;
}

class qTools : public QWidget
{
    Q_OBJECT

public:
    explicit qTools(QWidget *parent = nullptr);
    ~qTools();

private:
    Ui::qTools *ui;
};

#endif // QTOOLS_H
