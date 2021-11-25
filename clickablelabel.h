#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent) { Q_UNUSED (parent) }
protected:
    void mousePressEvent(QMouseEvent *event) { Q_UNUSED (event) emit clicked(); }
signals:
    void clicked();
};
