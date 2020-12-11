#ifndef UM_DIALOG_H
#define UM_DIALOG_H

#include <QWidget>

namespace Ui {
class UM_Dialog;
}

class UM_Dialog : public QWidget
{
    Q_OBJECT

public:
    explicit UM_Dialog(QWidget *parent = nullptr);
    ~UM_Dialog();

private:
    Ui::UM_Dialog *ui;
};

#endif // UM_DIALOG_H
