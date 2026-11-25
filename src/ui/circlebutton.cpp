#include "circlebutton.h"

#include <QPainter>

CircleButton::CircleButton(int diameter, QWidget *parent)
    : QAbstractButton(parent), m_diameter(diameter)
{
    setFixedSize(diameter, diameter);
    setFocusPolicy(Qt::NoFocus);
}

void CircleButton::setLit(bool lit)
{
    if (m_lit == lit)
        return;
    m_lit = lit;
    update();
}

void CircleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(m_lit ? QColor(0xee, 0x1c, 0x25) : QColor(0xb4, 0xb4, 0xb4));
    p.drawEllipse(rect().adjusted(1, 1, -1, -1));
}
