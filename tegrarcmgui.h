#ifndef TEGRARCMGUI_H
#define TEGRARCMGUI_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class TegraRcmGUI; }
QT_END_NAMESPACE

class TegraRcmGUI : public QMainWindow
{
    Q_OBJECT

public:
    TegraRcmGUI(QWidget *parent = nullptr);
    ~TegraRcmGUI();

private:
    Ui::TegraRcmGUI *ui;
};
#endif // TEGRARCMGUI_H
