#pragma once
#include <QTableWidget>
#include <QPair>

struct Arrow {
    QPair<int,int> from;   // (row, col)
    QPair<int,int> to;     // (row, col)
};

class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = nullptr)
        : QTableWidget(parent)
    {}

    // replace all arrows at once:
    void setArrows(const QList<Arrow>& arrows) {
        m_arrows = arrows;
        viewport()->update();
    }

    // add a single arrow:
    void addArrow(int r1, int c1, int r2, int c2) {
        m_arrows.append({{r1,c1}, {r2,c2}});
        viewport()->update();
    }

    // clear all arrows:
    void clearArrows() {
        m_arrows.clear();
        viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<Arrow> m_arrows;
};
