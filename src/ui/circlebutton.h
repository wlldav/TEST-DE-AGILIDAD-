#ifndef CIRCLEBUTTON_H
#define CIRCLEBUTTON_H

#include <QAbstractButton>

/**
 * @class CircleButton
 * @brief Boton circular que puede estar encendido (rojo) o apagado (gris).
 *
 * Se dibuja con QPainter, sin hojas de estilo ni imagenes, para mantener
 * el ejecutable liviano.
 */
class CircleButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit CircleButton(int diameter = 48, QWidget *parent = nullptr);

    bool isLit() const { return m_lit; }
    /** @brief Enciende o apaga el boton y lo repinta. */
    void setLit(bool lit);

    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override { return QSize(m_diameter, m_diameter); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_diameter;
    bool m_lit = false;
};

#endif // CIRCLEBUTTON_H
